/***************************************************************************//**
 * @file pit.c
 *
 * @see pit.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 17/12/2017
 *
 * @version 1.0
 *
 * @brief PIT (Programmable interval timer) driver.
 *
 * @details PIT (Programmable interval timer) driver. Used as the basic timer
 * source in the kernel. This driver provides basic access to the PIT.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <Cpu/cpu.h>              /* cpu_outb */
#include <IO/kernel_output.h>     /* kernel_printf */
#include <Interrupt/interrupts.h> /* register_interrupt, cpu_state,
                                   * stack_state, set_IRQ_mask,
                                   * kernel_interrupt_set_irq_eoi */
#include <Lib/stdint.h>           /* Generioc int types */
#include <Lib/stddef.h>           /* OS_RETURN_E */
#include <Time/time_management.h> /* kernel_timer_t */
#include <Sync/critical.h>        /* ENTER_CRITICAL, EXIT_CRITICAL */

/* UTK configuration file */
#include <config.h>

/* Header include */
#include <Drivers/pit.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/** @brief Keeps track on the PIT enabled state. */
static uint32_t disabled_nesting;

/** @brief Keeps track of the PIT tick frequency. */
static uint32_t tick_freq;

/** @brief PIT driver instance. */
kernel_timer_t pit_driver = {
    .get_frequency  = pit_get_frequency,
    .set_frequency  = pit_set_frequency,
    .enable         = pit_enable,
    .disable        = pit_disable,
    .set_handler    = pit_set_handler,
    .remove_handler = pit_remove_handler,
    .get_irq        = pit_get_irq
};

#if MAX_CPU_COUNT > 1
/** @brief Critical section spinlock. */
static spinlock_t lock = SPINLOCK_INIT_VALUE;
#endif

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initial PIT interrupt handler.
 *
 * @details PIT interrupt handler set at the initialization of the PIT.
 * Dummy routine setting EOI.
 *
 * @param[in, out] cpu_state The cpu registers before the interrupt.
 * @param[in] int_id The interrupt line that called the handler.
 * @param[in, out] stack_state The stack state before the interrupt.
 */
static void dummy_handler(cpu_state_t* cpu_state, address_t int_id,
                          stack_state_t* stack_state)
{
    (void)cpu_state;
    (void)int_id;
    (void)stack_state;

    /* EOI */
    kernel_interrupt_set_irq_eoi(PIT_IRQ_LINE);
}

OS_RETURN_E pit_init(void)
{
    OS_RETURN_E err;

    /* Init system times */
    disabled_nesting = 1;

    /* Set PIT frequency */
    err = pit_set_frequency(PIT_INIT_FREQ);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    /* Set PIT interrupt handler */
    err = kernel_interrupt_register_irq_handler(PIT_IRQ_LINE, dummy_handler);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    #if PIT_KERNEL_DEBUG == 1
    kernel_serial_debug("PIT Initialization\n");
    #endif

    /* Enable PIT IRQ */
    return pit_enable();
}

OS_RETURN_E pit_enable(void)
{
    uint32_t word;

    #if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(word, &lock);
    #else
    ENTER_CRITICAL(word);
    #endif

    if(disabled_nesting > 0)
    {
        --disabled_nesting;
    }
    if(disabled_nesting == 0)
    {
        #if PIT_KERNEL_DEBUG == 1
        kernel_serial_debug("Enable PIT\n");
        #endif
        #if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(word, &lock);
        #else
        EXIT_CRITICAL(word);
        #endif
        return kernel_interrupt_set_irq_mask(PIT_IRQ_LINE, 1);
    }

    #if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(word, &lock);
    #else
    EXIT_CRITICAL(word);
    #endif

    return OS_NO_ERR;
}

OS_RETURN_E pit_disable(void)
{
    OS_RETURN_E err;
    uint32_t    word;

    #if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(word, &lock);
    #else
    ENTER_CRITICAL(word);
    #endif

    if(disabled_nesting < UINT32_MAX)
    {
        ++disabled_nesting;
    }

    #if PIT_KERNEL_DEBUG == 1
    kernel_serial_debug("Disable PIT (%d)\n", disabled_nesting);
    #endif
    err = kernel_interrupt_set_irq_mask(PIT_IRQ_LINE, 0);

    #if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(word, &lock);
    #else
    EXIT_CRITICAL(word);
    #endif

    return err;
}

OS_RETURN_E pit_set_frequency(const uint32_t freq)
{
    OS_RETURN_E err;
    uint32_t    word;

    if(freq < PIT_MIN_FREQ || freq > PIT_MAX_FREQ)
    {
        return OS_ERR_OUT_OF_BOUND;
    }

    #if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(word, &lock);
    #else
    ENTER_CRITICAL(word);
    #endif

    /* Disable PIT IRQ */
    err = pit_disable();
    if(err != OS_NO_ERR)
    {
        #if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(word, &lock);
        #else
        EXIT_CRITICAL(word);
        #endif
        return err;
    }

    tick_freq  = freq;

    /* Set clock frequency */
    uint16_t tick_freq = (uint16_t)((uint32_t)PIT_QUARTZ_FREQ / freq);
    cpu_outb(PIT_COMM_SET_FREQ, PIT_COMM_PORT);
    cpu_outb(tick_freq & 0x00FF, PIT_DATA_PORT);
    cpu_outb(tick_freq >> 8, PIT_DATA_PORT);


    #if PIT_KERNEL_DEBUG == 1
    kernel_serial_debug("New PIT frequency set (%d)\n", freq);
    #endif

    #if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(word, &lock);
    #else
    EXIT_CRITICAL(word);
    #endif

    /* Enable PIT IRQ */
    return pit_enable();
}

uint32_t pit_get_frequency(void)
{
    return tick_freq;
}

OS_RETURN_E pit_set_handler(void(*handler)(
                                 cpu_state_t*,
                                 address_t,
                                 stack_state_t*
                                 ))
{
    OS_RETURN_E err;
    uint32_t    word;

    if(handler == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    #if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(word, &lock);
    #else
    ENTER_CRITICAL(word);
    #endif

    err = pit_disable();
    if(err != OS_NO_ERR)
    {
        #if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(word, &lock);
        #else
        EXIT_CRITICAL(word);
        #endif
        return err;
    }

    /* Remove the current handler */
    err = kernel_interrupt_remove_irq_handler(PIT_IRQ_LINE);
    if(err != OS_NO_ERR)
    {
        #if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(word, &lock);
        #else
        EXIT_CRITICAL(word);
        #endif
        pit_enable();
        return err;
    }

    err = kernel_interrupt_register_irq_handler(PIT_IRQ_LINE, handler);
    if(err != OS_NO_ERR)
    {
        #if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(word, &lock);
        #else
        EXIT_CRITICAL(word);
        #endif
        pit_enable();
        return err;
    }

    #if PIT_KERNEL_DEBUG == 1
    kernel_serial_debug("New PIT handler set (0x%08x)\n", handler);
    #endif

    #if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(word, &lock);
    #else
    EXIT_CRITICAL(word);
    #endif

    return pit_enable();
}

OS_RETURN_E pit_remove_handler(void)
{
    #if PIT_KERNEL_DEBUG == 1
    kernel_serial_debug("Default PIT handler set\n");
    #endif
    return pit_set_handler(dummy_handler);
}

uint32_t pit_get_irq(void)
{
    return PIT_IRQ_LINE;
}