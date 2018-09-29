/***************************************************************************//**
 * @file interrupts.c
 * 
 * @see interrupts.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 14/12/2017
 *
 * @version 1.5
 *
 * @brief X86 interrupt manager.
 * 
 * @details X86 interrupt manager. Allows to attach ISR to interrupt lines and
 * manage IRQ used by the CPU. We also define the general interrupt handler 
 * here.
 * 
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <Lib/stdint.h>       /* Generic int types */
#include <Lib/stddef.h>       /* OS_RETURN_E */
#include <Lib/string.h>       /* memset */
#include <BSP/pic.h>      /* pic_set_irq_eoi, pic_set_irq_mask */
#include <Cpu/cpu_settings.h> /* IDT_ENTRY_COUNT */
#include <Cpu/cpu.h>          /* sti cpu_cli */
#include <IO/kernel_output.h> /* kernel_success */
#include <Interrupt/panic.h>  /* panic () */

/* RTLK configuration file */
#include <config.h>

/* Header file */
#include <Interrupt/interrupts.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/** @brief Stores the handlers for each interrupt. */
custom_handler_t kernel_interrupt_handlers[IDT_ENTRY_COUNT];

/** @brief The current interrupt driver to be used by the kernel. */
static interrupt_driver_t interrupt_driver;

/** @brief Keep track of the current interrupt state. */
static uint32_t int_state;

/** @brief Stores the number of spurious interrupts since the initialization of
 * the kernel.
 */
static uint32_t spurious_interrupt;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/
/**
 * @brief Kernel's spurious interrupt handler.
 * 
 * @details Spurious interrupt handler. This function should only be 
 * called by an assembly interrupt handler. The function will handle spurious
 * interrupts.
 */
static void spurious_handler(void)
{
     #if INTERRUPT_KERNEL_DEBUG == 1
    kernel_serial_debug("Spurious interrupt\n");
    #endif

    ++spurious_interrupt;

    return;
}

/**
 * @brief Kernel's main interrupt handler.
 * 
 * @details Generic and global interrupt handler. This function should only be 
 * called by an assembly interrupt handler. The function will dispatch the 
 * interrupt to the desired function to handle the interrupt.
 *
 * @param[in, out] cpu_state The cpu registers structure.
 * @param[in] int_id The interrupt number.
 * @param[in, out] stack_state The stack state before the interrupt that contain cs, 
 * eip, error code and the eflags register value.
 */
void kernel_interrupt_handler(cpu_state_t cpu_state,
                            uint32_t int_id,
                            stack_state_t stack_state)
{
    int32_t irq_id;
    void(*handler)(cpu_state_t*, uint32_t, stack_state_t*);

    /* If interrupts are disabled */
    if(int_state == 0 &&
       int_id != PANIC_INT_LINE &&
       int_id != SCHEDULER_SW_INT_LINE &&
       int_id >= MIN_INTERRUPT_LINE)
    {
        #if INTERRUPT_KERNEL_DEBUG == 1
        kernel_serial_debug("Blocked interrupt %d\n",
                            int_id);
        #endif

        return;
    }

    /* Check for spurious interrupt */
    irq_id = int_id - INT_IRQ_OFFSET;
    if(irq_id >= 0)
    {
        if(interrupt_driver.driver_handle_spurious((uint32_t) irq_id) == 
           INTERRUPT_TYPE_SPURIOUS)
        {
            spurious_handler();
            return;
        }
    }

    /* Select custom handlers */
    if(int_id < IDT_ENTRY_COUNT &&
       kernel_interrupt_handlers[int_id].enabled == 1 &&
       kernel_interrupt_handlers[int_id].handler != NULL)
    {
        handler = kernel_interrupt_handlers[int_id].handler;
    }
    else
    {
        handler = panic;
    }

    /* Execute the handler */
    handler(&cpu_state, int_id, &stack_state);
}

OS_RETURN_E kernel_interrupt_init(const interrupt_driver_t driver)
{
    if(driver.driver_set_irq_eoi == NULL || 
       driver.driver_set_irq_mask == NULL ||
       driver.driver_handle_spurious == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* Blank custom interrupt handlers */
    memset(kernel_interrupt_handlers, 0,
           sizeof(custom_handler_t) * IDT_ENTRY_COUNT);

    /* Attach the special PANIC interrupt for when we don't know what to do */
    kernel_interrupt_handlers[PANIC_INT_LINE].enabled = 1;
    kernel_interrupt_handlers[PANIC_INT_LINE].handler = panic;

    /* Init state */
    kernel_interrupt_disable();
    spurious_interrupt = 0;

    /* Set interrupt driver */ 
    interrupt_driver = driver;

     #if INTERRUPT_KERNEL_DEBUG == 1
    kernel_serial_debug("Initialized interrupt manager.\n");
    #endif

    return OS_NO_ERR;
}

OS_RETURN_E kernel_interrupt_set_driver(const interrupt_driver_t driver)
{
    if(driver.driver_set_irq_eoi == NULL || 
       driver.driver_set_irq_mask == NULL ||
       driver.driver_handle_spurious == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    interrupt_driver = driver;

    #if INTERRUPT_KERNEL_DEBUG == 1
    kernel_serial_debug("Set new interrupt driver.\n");
    #endif

    return OS_NO_ERR;
}

OS_RETURN_E kernel_interrupt_register_handler(const uint32_t interrupt_line,
                                       void(*handler)(
                                             cpu_state_t*,
                                             uint32_t,
                                             stack_state_t*
                                             )
                                       )
{
    if(interrupt_line < MIN_INTERRUPT_LINE ||
       interrupt_line > MAX_INTERRUPT_LINE)
    {
        return OR_ERR_UNAUTHORIZED_INTERRUPT_LINE;
    }

    if(handler == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    if(kernel_interrupt_handlers[interrupt_line].handler != NULL)
    {
        return OS_ERR_INTERRUPT_ALREADY_REGISTERED;
    }

    kernel_interrupt_handlers[interrupt_line].handler = handler;
    kernel_interrupt_handlers[interrupt_line].enabled = 1;

    #if INTERRUPT_KERNEL_DEBUG == 1
    kernel_serial_debug("Added INT %d handler at 0x%08x\n",
                        interrupt_line, (uint32_t)handler);
    #endif

    return OS_NO_ERR;
}

OS_RETURN_E kernel_interrupt_remove_handler(const uint32_t interrupt_line)
{
    if(interrupt_line < MIN_INTERRUPT_LINE ||
       interrupt_line > MAX_INTERRUPT_LINE)
    {
        return OR_ERR_UNAUTHORIZED_INTERRUPT_LINE;
    }

    if(kernel_interrupt_handlers[interrupt_line].handler == NULL)
    {
        return OS_ERR_INTERRUPT_NOT_REGISTERED;
    }

    kernel_interrupt_handlers[interrupt_line].handler = NULL;
    kernel_interrupt_handlers[interrupt_line].enabled = 0;

    #if INTERRUPT_KERNEL_DEBUG == 1
    kernel_serial_debug("Removed INT %d handle\n", interrupt_line);
    #endif

    return OS_NO_ERR;
}

void kernel_interrupt_restore(const uint32_t prev_state)
{
    if(prev_state != 0)
    {
        #if INTERRUPT_KERNEL_DEBUG == 1
        kernel_serial_debug("--- Enabled HW INT ---\n");
        #endif

        int_state = 1;
        cpu_sti();
    }
}

uint32_t kernel_interrupt_disable(void)
{
    uint32_t old_state = kernel_interrupt_get_state();

    cpu_cli();
    int_state = 0;

    #if INTERRUPT_KERNEL_DEBUG == 1
    kernel_serial_debug("--- Disabled HW INT ---\n");
    #endif

    return old_state;
}

uint32_t kernel_interrupt_get_state(void)
{
    return ((cpu_save_flags() & CPU_EFLAGS_IF) != 0);
}


OS_RETURN_E kernel_interrupt_set_irq_mask(const uint32_t irq_number, 
                                          const uint8_t enabled)
{
    #if INTERRUPT_KERNEL_DEBUG == 1
    kernel_serial_debug("IRQ Mask change: %d %d\n", irq_number, enabled);
    #endif
    return interrupt_driver.driver_set_irq_mask(irq_number, enabled);
}

OS_RETURN_E kernel_interrupt_set_irq_eoi(const uint32_t irq_number)
{
    #if INTERRUPT_KERNEL_DEBUG == 1
    kernel_serial_debug("IRQ EOI: %d\n", irq_number);
    #endif
    return interrupt_driver.driver_set_irq_eoi(irq_number);
}


