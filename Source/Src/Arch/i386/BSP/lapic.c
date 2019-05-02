/***************************************************************************//**
 * @file lapic.c
 *
 * @see lapic.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 25/12/2017
 *
 * @version 1.0
 *
 * @brief Local APIC (Advanced programmable interrupt controler) driver.
 *
 * @details Local APIC (Advanced programmable interrupt controler) driver.
 * Manages x86 IRQs from the IO-APIC. The driver also allow the use of the LAPIC
 * timer as a timer source. IPI (inter processor interrupt) are also possible
 * thanks to the driver.
 *
 * @warning This driver uses the PIT (Programmable interval timer) to initialize
 * the LAPIC timer. the PIC must be present and initialized to use this driver.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <Cpu/cpu.h>              /* mapped_io */
#include <Lib/stdint.h>           /* Generic int types */
#include <Lib/stddef.h>           /* OS_RETURN_E, NULL */
#include <Interrupt/interrupts.h> /* SPURIOUS_INTERRUPT_LINE,
                                   * register_interrupt_line */
#include <Cpu/cpu.h>              /* mapped_io_read_32, mapped_io_write_32 */
#include <BSP/acpi.h>             /* get_lapic_addr */
#include <BSP/pit.h>              /* set_pit_freq, emable_pit, diable_pit */
#include <Time/time_management.h> /* kernel_timer_t */
#include <Sync/critical.h>        /* ENTER_CRITICAL, EXIT_CRITICAL */
#include <Memory/paging.h>        /* kernel_mmap */
#include <Memory/paging_alloc.h>  /* kernel_paging_alloc_page */

/* RTLK configuration file */
#include <config.h>

/* Header file */
#include <BSP/lapic.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/* Local APIC controller address */
static void* lapic_base_addr;

/* Lapic TIMER settings */
static volatile uint8_t  wait_int;
static uint32_t global_lapic_freq;
static uint32_t init_lapic_timer_frequency;

/** @brief LAPIC timer driver instance. */
kernel_timer_t lapic_timer_driver = {
    .get_frequency  = lapic_timer_get_frequency,
    .set_frequency  = lapic_timer_set_frequency,
    .enable         = lapic_timer_enable,
    .disable        = lapic_timer_disable,
    .set_handler    = lapic_timer_set_handler,
    .remove_handler = lapic_timer_remove_handler,
    .get_irq        = lapic_timer_get_irq
};

#if MAX_CPU_COUNT > 1
/** @brief IPI critical section spinlock. */
static spinlock_t ipi_lock = SPINLOCK_INIT_VALUE;
#endif

#if MAX_CPU_COUNT > 1
/** @brief Timer critical section spinlock. */
static spinlock_t timer_lock[MAX_CPU_COUNT];
#endif


/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Read Local APIC register, the acces is a memory mapped IO.
 *
 * @param reg The register of the Local APIC to read.
 * @return The value contained in the Local APIC register.
 */
__inline__ static uint32_t lapic_read(uint32_t reg)
{
    return mapped_io_read_32((void*)((uint32_t)lapic_base_addr + reg));
}

/* Write Local APIC register, the acces is a memory mapped IO.
 *
 * @param reg The register of the Local APIC to write.
 * @param data The value to write in the register.
 */
__inline__ static void lapic_write(uint32_t reg, uint32_t data)
{
    mapped_io_write_32((void*)((uint32_t)lapic_base_addr + reg), data);
}

/* LAPIC dummy hamdler.
 * @param cpu_state The cpu registers structure.
 * @param int_id The interrupt number.
 * @param stack_state The stack state before the interrupt that contain cs, eip,
 * error code and the eflags register value.
 */
static void lapic_dummy_handler(cpu_state_t* cpu_state, uint32_t int_id,
                                stack_state_t* stack_state)
{
    (void)cpu_state;
    (void)int_id;
    (void)stack_state;

    kernel_interrupt_set_irq_eoi(LAPIC_TIMER_INTERRUPT_LINE);
}

/* Initialisation handler. The PIT will trigger two interrupts to init the
 * LAPIC timer. This is used to get the LAPIC timer frequency.
 *
 * @param cpu_state The cpu registers structure.
 * @param int_id The interrupt number.
 * @param stack_state The stack state before the interrupt that contain cs, eip,
 * error code and the eflags register value.
 */
static void lapic_init_pit_handler(cpu_state_t* cpu_state, uint32_t int_id,
                                   stack_state_t* stack_state)
{
    (void)cpu_state;
    (void)int_id;
    (void)stack_state;

    if(wait_int == 1)
    {
        ++wait_int;
        /* Set LAPIC init counter to -1 */
        lapic_write(LAPIC_TICR, 0xFFFFFFFF);
    }
    else if(wait_int == 2)
    {
        /* Stop the LAPIC timer */
        lapic_write(LAPIC_TIMER, LAPIC_LVT_INT_MASKED);
        wait_int = 0;
    }

    kernel_interrupt_set_irq_eoi(PIT_IRQ_LINE);
}

OS_RETURN_E lapic_init(void)
{
    OS_RETURN_E err;

    #if MAX_CPU_COUNT > 1
    uint32_t    i;
    #endif

    const void* lapic_phys_addr;

    #if LAPIC_KERNEL_DEBUG == 1
    kernel_serial_debug("LAPIC Initialization\n");
    #endif
    /* Check IO-APIC support */
    #if ENABLE_IO_APIC == 0
    return OS_ERR_NOT_SUPPORTED;
    #endif
    if(acpi_get_io_apic_available() == 0 || acpi_get_lapic_available() == 0)
    {
        return OS_ERR_NOT_SUPPORTED;
    }

    #if MAX_CPU_COUNT > 1
    for(i = 0; i < MAX_CPU_COUNT; ++i)
    {
        INIT_SPINLOCK(&timer_lock[i]);
    }
    #endif

    /* Get Local APIC base address */
    lapic_phys_addr = acpi_get_lapic_addr();

    /* Get a free page */
    lapic_base_addr = kernel_paging_alloc_pages(1, &err);
    if(lapic_base_addr == NULL)
    {
        return err;
    }

    /* Map the LAPIC */
    err = kernel_direct_mmap(lapic_base_addr, lapic_phys_addr, 1,
                             PG_DIR_FLAG_PAGE_SIZE_4KB |
                             PG_DIR_FLAG_PAGE_SUPER_ACCESS |
                             PG_DIR_FLAG_PAGE_READ_WRITE,
                             1);
    if(err != OS_NO_ERR)
    {
        kernel_paging_free_pages((void*)lapic_base_addr, 1);
        return err;
    }

    /* Add offset */
    lapic_base_addr = (void*)((uint32_t)lapic_base_addr +
                             ((uint32_t)lapic_phys_addr & 0xFFF));

    /* Enable all interrupts */
    lapic_write(LAPIC_TPR, 0);

    /* Set logical destination mode */
    lapic_write(LAPIC_DFR, 0xffffffff);
    lapic_write(LAPIC_LDR, 0x01000000);

    /* Spurious Interrupt Vector Register */
    lapic_write(LAPIC_SVR, 0x100 | LAPIC_SPURIOUS_INT_LINE);

    return OS_NO_ERR;
}

int32_t lapic_get_id(void)
{
    /* Check IO-APIC support */
    #if ENABLE_IO_APIC == 0
    return -1;
    #endif
    if(acpi_get_io_apic_available() != 1 || acpi_get_lapic_available() != 1)
    {
        return -1;
    }

    return (lapic_read(LAPIC_ID) >> 24);
}

OS_RETURN_E lapic_send_ipi_init(const uint32_t lapic_id)
{
    OS_RETURN_E err;
    uint32_t    word;

    #if LAPIC_KERNEL_DEBUG == 1
    kernel_serial_debug("LAPIC Send INIT IPI\n");
    #endif

    #if LAPIC_KERNEL_DEBUG == 1
    kernel_serial_debug("LAPIC Initialization\n");
    #endif

    /* Check IO-APIC support */
    #if ENABLE_IO_APIC == 0
    return OS_ERR_NOT_SUPPORTED;
    #endif
    if(acpi_get_io_apic_available() == 0 || acpi_get_lapic_available() == 0)
    {
        return OS_ERR_NOT_SUPPORTED;
    }

    #if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(word, &ipi_lock);
    #else
    ENTER_CRITICAL(word);
    #endif

    /* Check LACPI id */
    err = acpi_check_lapic_id(lapic_id);
    if(err != OS_NO_ERR)
    {
        #if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(word, &ipi_lock);
        #else
        EXIT_CRITICAL(word);
        #endif
        return err;
    }

    /* Send IPI */
    lapic_write(LAPIC_ICRHI, lapic_id << ICR_DESTINATION_SHIFT);
    lapic_write(LAPIC_ICRLO, ICR_INIT | ICR_PHYSICAL |
                ICR_ASSERT | ICR_EDGE | ICR_NO_SHORTHAND);

    /* Wait for pending sends */
    while ((lapic_read(LAPIC_ICRLO) & ICR_SEND_PENDING) != 0)
    {}

    #if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(word, &ipi_lock);
    #else
    EXIT_CRITICAL(word);
    #endif

    return err;
}

OS_RETURN_E lapic_send_ipi_startup(const uint32_t lapic_id,
                                   const uint32_t vector)
{
    OS_RETURN_E err;
    uint32_t    word;

    #if LAPIC_KERNEL_DEBUG == 1
    kernel_serial_debug("LAPIC Send STARTUP IPI\n");
    #endif

    #if LAPIC_KERNEL_DEBUG == 1
    kernel_serial_debug("LAPIC Initialization\n");
    #endif
    /* Check IO-APIC support */
    #if ENABLE_IO_APIC == 0
    return OS_ERR_NOT_SUPPORTED;
    #endif
    if(acpi_get_io_apic_available() == 0 || acpi_get_lapic_available() == 0)
    {
        return OS_ERR_NOT_SUPPORTED;
    }

    #if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(word, &ipi_lock);
    #else
    ENTER_CRITICAL(word);
    #endif

    /* Check LACPI id */
    err = acpi_check_lapic_id(lapic_id);
    if(err != OS_NO_ERR)
    {
        #if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(word, &ipi_lock);
        #else
        EXIT_CRITICAL(word);
        #endif
        return err;
    }

    /* Send IPI */
    lapic_write(LAPIC_ICRHI, lapic_id << ICR_DESTINATION_SHIFT);
    lapic_write(LAPIC_ICRLO, (vector & 0xFF) | ICR_STARTUP | ICR_PHYSICAL |
                ICR_ASSERT | ICR_EDGE | ICR_NO_SHORTHAND);

    /* Wait for pending sends */
    while ((lapic_read(LAPIC_ICRLO) & ICR_SEND_PENDING) != 0)
    {}

    #if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(word, &ipi_lock);
    #else
    EXIT_CRITICAL(word);
    #endif

    return err;
}

OS_RETURN_E lapic_send_ipi(const uint32_t lapic_id, const uint32_t vector)
{
    OS_RETURN_E err;
    uint32_t    word;

    #if LAPIC_KERNEL_DEBUG == 1
    kernel_serial_debug("LAPIC Send IPI\n");
    #endif

    #if LAPIC_KERNEL_DEBUG == 1
    kernel_serial_debug("LAPIC Initialization\n");
    #endif
    /* Check IO-APIC support */
    #if ENABLE_IO_APIC == 0
    return OS_ERR_NOT_SUPPORTED;
    #endif
    if(acpi_get_io_apic_available() == 0 || acpi_get_lapic_available() == 0)
    {
        return OS_ERR_NOT_SUPPORTED;
    }

    #if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(word, &ipi_lock);
    #else
    ENTER_CRITICAL(word);
    #endif

    /* Check LACPI id */
    err = acpi_check_lapic_id(lapic_id);
    if(err != OS_NO_ERR)
    {
        #if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(word, &ipi_lock);
        #else
        EXIT_CRITICAL(word);
        #endif

        return err;
    }

    /* Send IPI */
    lapic_write(LAPIC_ICRHI, lapic_id << ICR_DESTINATION_SHIFT);
    lapic_write(LAPIC_ICRLO, (vector & 0xFF) | ICR_PHYSICAL |
                ICR_ASSERT | ICR_EDGE | ICR_NO_SHORTHAND);

    /* Wait for pending sends */
    while ((lapic_read(LAPIC_ICRLO) & ICR_SEND_PENDING) != 0)
    {}

    #if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(word, &ipi_lock);
    #else
    EXIT_CRITICAL(word);
    #endif

    return err;
}

OS_RETURN_E lapic_set_int_eoi(const uint32_t interrupt_line)
{
    if(interrupt_line > MAX_INTERRUPT_LINE)
    {
        return OS_ERR_NO_SUCH_IRQ_LINE;
    }

    lapic_write(LAPIC_EOI, 0);

    #if LAPIC_KERNEL_DEBUG == 1
    kernel_serial_debug("LAPIC EOI %d \n", interrupt_line);
    #endif

    return OS_NO_ERR;
}

OS_RETURN_E lapic_timer_init(void)
{
    uint32_t    lapic_timer_tick_10ms;
    OS_RETURN_E err;

    #if LAPIC_KERNEL_DEBUG == 1
    kernel_serial_debug("LAPIC Timer Initialization\n");
    #endif

    #if LAPIC_KERNEL_DEBUG == 1
    kernel_serial_debug("LAPIC Initialization\n");
    #endif
    /* Check IO-APIC support */
    #if ENABLE_IO_APIC == 0
    return OS_ERR_NOT_SUPPORTED;
    #endif
    if(acpi_get_io_apic_available() == 0 || acpi_get_lapic_available() == 0)
    {
        return OS_ERR_NOT_SUPPORTED;
    }

    /* Init LAPIC TIMER */
    wait_int = 1;
    lapic_write(LAPIC_TDCR, LAPIC_DIVIDER_16);

    /* Set PIT period of 10 ms and handler */
    err = pit_set_frequency(100);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    err = pit_set_handler(lapic_init_pit_handler);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    /* Wait for interrupts to gather the timer data */
    err = pit_enable();
    if(err != OS_NO_ERR)
    {
        return err;
    }

    kernel_interrupt_restore(1);
    while(wait_int != 0);
    kernel_interrupt_disable();

    err = pit_disable();
    if(err != OS_NO_ERR)
    {
        return err;
    }

    err = pit_remove_handler();
    if(err != OS_NO_ERR)
    {
        return err;
    }

    /* Get the count of ticks in 10ms */
    lapic_timer_tick_10ms = 0xFFFFFFFF - lapic_read(LAPIC_TCCR);

    /* Get the frequency */
    init_lapic_timer_frequency = 100 * lapic_timer_tick_10ms;

    /* Compute the new tick count */
    global_lapic_freq = init_lapic_timer_frequency / LAPIC_INIT_FREQ;

    /* Register LAPI dummy handler */
    err = kernel_interrupt_register_int_handler(LAPIC_TIMER_INTERRUPT_LINE,
                                                lapic_dummy_handler);
    if(err != OS_NO_ERR)
    {
        return err;
    }
    /* Init interrupt */
    lapic_write(LAPIC_TIMER, LAPIC_TIMER_INTERRUPT_LINE |
                LAPIC_TIMER_MODE_PERIODIC);

    /* Set new timer count */
    lapic_write(LAPIC_TDCR, LAPIC_DIVIDER_16);
    lapic_write(LAPIC_TICR, global_lapic_freq);

    lapic_set_int_eoi(LAPIC_TIMER_INTERRUPT_LINE);

    return err;
}

OS_RETURN_E lapic_ap_timer_init(void)
{
    uint32_t word;

    #if MAX_CPU_COUNT > 1
    int32_t cpu_id;

    cpu_id = lapic_get_id();
    #endif

    #if LAPIC_KERNEL_DEBUG == 1
    kernel_serial_debug("LAPIC Timer AP Initialization\n");
    #endif

    #if LAPIC_KERNEL_DEBUG == 1
    kernel_serial_debug("LAPIC Initialization\n");
    #endif
    /* Check IO-APIC support */
    #if ENABLE_IO_APIC == 0
    return OS_ERR_NOT_SUPPORTED;
    #endif
    if(acpi_get_io_apic_available() == 0 || acpi_get_lapic_available() == 0)
    {
        return OS_ERR_NOT_SUPPORTED;
    }

    #if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(word, &timer_lock[cpu_id]);
    #else
    ENTER_CRITICAL(word);
    #endif

    /* Init LAPIC TIMER */
    lapic_write(LAPIC_TDCR, LAPIC_DIVIDER_16);

    /* Init interrupt */
    lapic_write(LAPIC_TIMER, LAPIC_TIMER_INTERRUPT_LINE |
                LAPIC_TIMER_MODE_PERIODIC);

    /* Set new timer count */
    lapic_write(LAPIC_TDCR, LAPIC_DIVIDER_16);
    lapic_write(LAPIC_TICR, global_lapic_freq);

    #if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(word, &timer_lock[cpu_id]);
    #else
    EXIT_CRITICAL(word);
    #endif

    return OS_NO_ERR;
}

uint32_t lapic_timer_get_frequency(void)
{
    uint32_t freq;
    uint32_t word;

    #if MAX_CPU_COUNT > 1
    int32_t cpu_id;

    cpu_id = lapic_get_id();
    #endif

    #if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(word, &timer_lock[cpu_id]);
    #else
    ENTER_CRITICAL(word);
    #endif

    freq = init_lapic_timer_frequency / global_lapic_freq;

    #if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(word, &timer_lock[cpu_id]);
    #else
    EXIT_CRITICAL(word);
    #endif

    return freq;
}

OS_RETURN_E lapic_timer_set_frequency(const uint32_t frequency)
{
    uint32_t word;

    #if MAX_CPU_COUNT > 1
    int32_t cpu_id;

    cpu_id = lapic_get_id();
    #endif 

    #if LAPIC_KERNEL_DEBUG == 1
    kernel_serial_debug("LAPIC Timer set frequency %d\n", frequency);
    #endif

    #if LAPIC_KERNEL_DEBUG == 1
    kernel_serial_debug("LAPIC Initialization\n");
    #endif
    /* Check IO-APIC support */
    #if ENABLE_IO_APIC == 0
    return OS_ERR_NOT_SUPPORTED;
    #endif
    if(acpi_get_io_apic_available() == 0 || acpi_get_lapic_available() == 0)
    {
        return OS_ERR_NOT_SUPPORTED;
    }

    #if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(word, &timer_lock[cpu_id]);
    #else
    ENTER_CRITICAL(word);
    #endif

    /* Compute the new tick count */
    global_lapic_freq = init_lapic_timer_frequency / frequency;

    /* Set new timer count */
    lapic_write(LAPIC_TDCR, LAPIC_DIVIDER_16);
    lapic_write(LAPIC_TICR, global_lapic_freq);

    #if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(word, &timer_lock[cpu_id]);
    #else
    EXIT_CRITICAL(word);
    #endif

    return OS_NO_ERR;
}

OS_RETURN_E lapic_timer_enable(void)
{
    uint32_t word;

    #if MAX_CPU_COUNT > 1
    int32_t cpu_id;

    cpu_id = lapic_get_id();
    #endif

    #if LAPIC_KERNEL_DEBUG == 1
    kernel_serial_debug("LAPIC Timer enable\n");
    #endif

    #if LAPIC_KERNEL_DEBUG == 1
    kernel_serial_debug("LAPIC Initialization\n");
    #endif
    /* Check IO-APIC support */
    #if ENABLE_IO_APIC == 0
    return OS_ERR_NOT_SUPPORTED;
    #endif
    if(acpi_get_io_apic_available() == 0 || acpi_get_lapic_available() == 0)
    {
        return OS_ERR_NOT_SUPPORTED;
    }

    #if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(word, &timer_lock[cpu_id]);
    #else
    ENTER_CRITICAL(word);
    #endif

    /* Enable interrupt */
    lapic_write(LAPIC_TIMER, LAPIC_TIMER_INTERRUPT_LINE |
                LAPIC_TIMER_MODE_PERIODIC);

    #if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(word, &timer_lock[cpu_id]);
    #else
    EXIT_CRITICAL(word);
    #endif

    return OS_NO_ERR;
}

OS_RETURN_E lapic_timer_disable(void)
{
    uint32_t word;

    #if MAX_CPU_COUNT > 1
    int32_t cpu_id;

    cpu_id = lapic_get_id();
    #endif

    #if LAPIC_KERNEL_DEBUG == 1
    kernel_serial_debug("LAPIC Timer disable\n");
    #endif

    #if LAPIC_KERNEL_DEBUG == 1
    kernel_serial_debug("LAPIC Initialization\n");
    #endif
    /* Check IO-APIC support */
    #if ENABLE_IO_APIC == 0
    return OS_ERR_NOT_SUPPORTED;
    #endif
    if(acpi_get_io_apic_available() == 0 || acpi_get_lapic_available() == 0)
    {
        return OS_ERR_NOT_SUPPORTED;
    }

    #if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(word, &timer_lock[cpu_id]);
    #else
    ENTER_CRITICAL(word);
    #endif

    /* Disable interrupt */
    lapic_write(LAPIC_TIMER, LAPIC_LVT_INT_MASKED);

    #if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(word, &timer_lock[cpu_id]);
    #else
    EXIT_CRITICAL(word);
    #endif

    return OS_NO_ERR;
}

OS_RETURN_E lapic_timer_set_handler(void(*handler)(
                                    cpu_state_t*,
                                    uint32_t,
                                    stack_state_t*
                                    ))
{
    OS_RETURN_E err;
    uint32_t    word;

    #if MAX_CPU_COUNT > 1
    int32_t cpu_id;

    cpu_id = lapic_get_id();
    #endif

    #if LAPIC_KERNEL_DEBUG == 1
    kernel_serial_debug("LAPIC Initialization\n");
    #endif
    /* Check IO-APIC support */
    #if ENABLE_IO_APIC == 0
    return OS_ERR_NOT_SUPPORTED;
    #endif
    if(acpi_get_io_apic_available() == 0 || acpi_get_lapic_available() == 0)
    {
        return OS_ERR_NOT_SUPPORTED;
    }

    if(handler == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    err = lapic_timer_disable();
    if(err != OS_NO_ERR)
    {
        return err;
    }

    #if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(word, &timer_lock[cpu_id]);
    #else
    ENTER_CRITICAL(word);
    #endif

    /* Remove the current handler */
    err = kernel_interrupt_remove_int_handler(LAPIC_TIMER_INTERRUPT_LINE);
    if(err != OS_NO_ERR)
    {
        #if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(word, &timer_lock[cpu_id]);
        #else
        EXIT_CRITICAL(word);
        #endif
        lapic_timer_enable();
        return err;
    }

    err = kernel_interrupt_register_int_handler(LAPIC_TIMER_INTERRUPT_LINE,
                                                handler);
    if(err != OS_NO_ERR)
    {
        #if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(word, &timer_lock[cpu_id]);
        #else
        EXIT_CRITICAL(word);
        #endif
        return err;
    }

    #if LAPIC_KERNEL_DEBUG == 1
    kernel_serial_debug("New LAPIC handler set (0x%08x)\n", handler);
    #endif

    #if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(word, &timer_lock[cpu_id]);
    #else
    EXIT_CRITICAL(word);
    #endif

    return lapic_timer_enable();
}

OS_RETURN_E lapic_timer_remove_handler(void)
{
    #if LAPIC_KERNEL_DEBUG == 1
    kernel_serial_debug("LAPIC Initialization\n");
    #endif
    /* Check IO-APIC support */
    #if ENABLE_IO_APIC == 0
    return OS_ERR_NOT_SUPPORTED;
    #endif
    if(acpi_get_io_apic_available() == 0 || acpi_get_lapic_available() == 0)
    {
        return OS_ERR_NOT_SUPPORTED;
    }

    return lapic_timer_set_handler(lapic_dummy_handler);
}

uint32_t lapic_timer_get_irq(void)
{
    return LAPIC_TIMER_INTERRUPT_LINE;
}