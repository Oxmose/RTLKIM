/***************************************************************************//**
 * @file interrupts.h
 * 
 * @see interrupts.c
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

#ifndef __INTERRUPTS_H_
#define __INTERRUPTS_H_

#include <Lib/stdint.h>       /* Generic int types */
#include <Lib/stddef.h>       /* OS_RETURN_E */
#include <Cpu/cpu_settings.h> /* IDT_ENTRY_COUNT */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Offset of the first line of an IRQ interrupt. */
#define INT_IRQ_OFFSET     0x30
/** @brief Minimal customizable accepted interrupt line. */
#define MIN_INTERRUPT_LINE 0x20
/** @brief Maximal customizable accepted interrupt line. */
#define MAX_INTERRUPT_LINE (IDT_ENTRY_COUNT - 2)

/** @brief PIT IRQ number. */
#define PIT_IRQ_LINE              0
/** @brief PIT interrupt line. */
#define PIT_INTERRUPT_LINE        (INT_IRQ_OFFSET + PIT_IRQ_LINE)
/** @brief Keyboard IRQ number. */
#define KBD_IRQ_LINE              1
/** @brief Keyboard interrupt line. */
#define KBD_INTERRUPT_LINE        (INT_IRQ_OFFSET + KBD_IRQ_LINE)
/** @brief Serial COM2-4 IRQ number. */
#define SERIAL_2_4_IRQ_LINE       3
/** @brief Serial COM2-4 interrupt line. */
#define SERIAL_2_4_INTERRUPT_LINE (INT_IRQ_OFFSET + SERIAL_2_4_IRQ_LINE)
/** @brief Serial COM1-3 IRQ number. */
#define SERIAL_1_3_IRQ_LINE       4
/** @brief Serial COM1-3 interrupt line. */
#define SERIAL_1_3_INTERRUPT_LINE (INT_IRQ_OFFSET + SERIAL_1_3_IRQ_LINE)
/** @brief RTC IRQ number. */
#define RTC_IRQ_LINE              8
/** @brief RTC interrupt line. */
#define RTC_INTERRUPT_LINE        (INT_IRQ_OFFSET + RTC_IRQ_LINE)
/** @brief Mouse IRQ number. */
#define MOUSE_IRQ_LINE            12
/** @brief Mouse interrupt line. */
#define MOUSE_INTERRUPT_LINE      (INT_IRQ_OFFSET + MOUSE_IRQ_LINE)

/** @brief LAPIC Timer interrupt line. */
#define LAPIC_TIMER_INTERRUPT_LINE 0x20
/** @brief Scheduler software interrupt line. */
#define SCHEDULER_SW_INT_LINE      0x21
/** @brief Panic software interrupt line. */
#define PANIC_INT_LINE             0x2A
/** @brief Spurious interrupt line. */
#define SPURIOUS_INT_LINE          (IDT_ENTRY_COUNT - 1)

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/** @brief Holds the CPU register values */
struct cpu_state
{
    /** @brief CPU's esp register. */
    uint32_t esp;
    /** @brief CPU's ebp register. */
    uint32_t ebp;
    /** @brief CPU's edi register. */
    uint32_t edi;
    /** @brief CPU's esi register. */
    uint32_t esi;
    /** @brief CPU's edx register. */
    uint32_t edx;
    /** @brief CPU's ecx register. */
    uint32_t ecx;
    /** @brief CPU's ebx register. */
    uint32_t ebx;
    /** @brief CPU's eax register. */
    uint32_t eax;
    
    /** @brief CPU's ss register. */
    uint32_t ss;
    /** @brief CPU's gs register. */
    uint32_t gs;
    /** @brief CPU's fs register. */
    uint32_t fs;
    /** @brief CPU's es register. */
    uint32_t es;
    /** @brief CPU's ds register. */
    uint32_t ds;
} __attribute__((packed));

/** 
 * @brief Defines cpu_state_t type as a shorcut for struct cpu_state.
 */
typedef struct cpu_state cpu_state_t;

/** @brief Hold the stack state before the interrupt */
struct stack_state
{
    /** @brief Interrupt's error code. */
    uint32_t error_code;
    /** @brief EIP of the faulting instruction. */
    uint32_t eip;
    /** @brief CS before the interrupt. */
    uint32_t cs;
    /** @brief EFLAGDS before the interrupt. */
    uint32_t eflags;
} __attribute__((packed));

/** 
 * @brief Defines stack_state_t type as a shorcut for struct stack_state.
 */
typedef struct stack_state stack_state_t;

/** @brief Custom interrupt handler structure. */
struct custom_handler
{
    /** @brief Handler's state.*/
    int32_t  enabled;
    /** @brief Handler's entry point. */
    void(*handler)(cpu_state_t*, uint32_t, stack_state_t*);
};

/** 
 * @brief Defines custom_handler_t type as a shorcut for struct custom_handler.
 */
typedef struct custom_handler custom_handler_t;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Kernel's main interrupt handler.
 * 
 * @details Generic and global interrupt handler. This function should only be 
 * called by an assembly interrupt handler. The function will dispatch the 
 * interrupt to the desired function to handle the interrupt.
 *
 * @param[in] cpu_state The cpu registers structure.
 * @param[in] int_id The interrupt number.
 * @param[in] stack_state The stack state before the interrupt that contain cs, 
 * eip, error code and the eflags register value.
 */
void kernel_interrupt_handler(cpu_state_t cpu_state,
                              uint32_t int_id,
                              stack_state_t stack_state);

/**
 * @brief Initializes the kernel's interrupt manager.
 * 
 * @details Blanks the handlers memory, initializes panic and spurious interrupt 
 * lines handlers.
 * 
 * @return The succes state or the error code. 
 * - OS_NO_ERR is returned if no error is encountered. 
 * - No other return value is possible.
 */
OS_RETURN_E init_kernel_interrupt(void);

/**
 * @brief Registers a new interrupt handler for the desired interrupt line.
 * 
 * @details Registers a custom interrupt handler to be executed. The interrupt 
 * line must be greater or equal to the minimal authorized custom interrupt line 
 * and less than the maximal one.
 *
 * @param[in] interrupt_line The interrupt line to attach the handler to.
 * @param[in] handler The handler for the desired interrupt.
 * 
 * @return The succes state or the error code. 
 * - OS_NO_ERR is returned if no error is encountered. 
 * - OR_ERR_UNAUTHORIZED_INTERRUPT_LINE is returned if the desired
 * interrupt line is not allowed. 
 * - OS_ERR_NULL_POINTER is returned if the pointer
 * to the handler is NULL. 
 * - OS_ERR_INTERRUPT_ALREADY_REGISTERED is returned if a 
 * handler is already registered for this interrupt line.
 */
OS_RETURN_E register_interrupt_handler(const uint32_t interrupt_line,
                                       void(*handler)(
                                             cpu_state_t*,
                                             uint32_t,
                                             stack_state_t*
                                             )
                                       );

/**
 * @brief Unregisters a new interrupt handler for the desired interrupt line.
 * 
 * @details Unregisters a custom interrupt handler to be executed. The interrupt 
 * line must be greater or equal to the minimal authorized custom interrupt line 
 * and less than the maximal one.
 *
 * @param[in] interrupt_line The interrupt line to deattach the handler from.
 * 
 * @return The succes state or the error code. 
 * - OS_NO_ERR is returned if no error is encountered. 
 * - OR_ERR_UNAUTHORIZED_INTERRUPT_LINE is returned if the desired
 * interrupt line is not allowed.
 * - OS_ERR_INTERRUPT_NOT_REGISTERED is returned if the interrupt line has no
 * handler attached.
 */
OS_RETURN_E remove_interrupt_handler(const uint32_t interrupt_line);

/**
 * @brief Registers a custom exception handler to be executed.
 * 
 * @details Registers a custom exception handler to be executed. If a handler 
 * was already set it will be overwritten.
 * 
 * @param[in] exception_line The exception line to attach the handler to.
 * @param[in] handler The handler for the desired exception.
 * 
 * @return The succes state or the error code. 
 * - OS_NO_ERR is returned if no error is encountered. 
 * - OR_ERR_UNAUTHORIZED_INTERRUPT_LINE is returned if the desired
 * interrupt line is not allowed. 
 * - OS_ERR_NULL_POINTER is returned if the pointer
 * to the handler is NULL.
 */
OS_RETURN_E register_exception_handler(const uint32_t exception_line,
                                       void(*handler)(
                                             cpu_state_t*,
                                             uint32_t,
                                             stack_state_t*
                                             )
                                       );

/**
 * @brief Restores the CPU interrupts state.
 * 
 * @details Restores the CPU interrupts state by setting the EFLAGS accordingly.
 *
 * @param[in] prev_state The previous interrupts state that has to be retored.
 */
void restore_local_interrupt(const uint32_t prev_state);

/* Disable CPU interrupt (SW/HW)
 * We keep track of the interrupt state nesting and disable interrupts in all
 * cases.
 *
 * @returns  The interupt state prior to disabling interrupts, to be used with
 * restore_local_interrupt
 */

/**
 * @brief Disables the CPU interrupts.
 * 
 * @details Disables the CPU interrupts by setting the EFLAGS accordingly.
 *
 * @return The current interrupt state is returned to be restored latter in the
 * execution of the kernel.
 */
uint32_t disable_local_interrupt(void);

/** 
 * @brief Tells if the interrupts are enabled for the current CPU.
 * 
 * @details Tells if the interrupts are enabled for the current CPU.
 *
 * @return The functions returns 1 if the interrupts are enabled, every other
 * values are considered as false.
 */
uint32_t get_local_interrupt_enabled(void);

/**
 * @brief Sets the IRQ mask for the IRQ number given as parameter.
 * 
 * @details Sets the IRQ mask for the IRQ number given as parameter.
 *
 * @param[in] irq_number The irq number to enable/disable.
 * @param[in] enabled Must be set to 1 to enable the IRQ or 0 to disable the 
 * IRQ.
 * 
 * @return The succes state or the error code. 
 * - OS_NO_ERR is returned if no error is encountered. 
 * - OS_ERR_NO_SUCH_IRQ_LINE is returned if the desired IRQ is not allowed. 
 */
OS_RETURN_E set_IRQ_mask(const uint32_t irq_number, const uint8_t enabled);

/**
 * @brief Acknowleges an IRQ.
 *
 * @details Acknowleges an IRQ.
 * 
 * @param[in] irq_number The irq number to acknowledge.
 * 
 * @return The succes state or the error code. 
 * - OS_NO_ERR is returned if no error is encountered. 
 * - OS_ERR_NO_SUCH_IRQ_LINE is returned if the desired IRQ is not allowed. 
 */
OS_RETURN_E set_IRQ_EOI(const uint32_t irq_number);

/**
 * @brief Updates the kernel time counter by one tick.
 * 
 * @details Updates the kernel time counter by one tick and computes the uptime 
 * in ms.
 */
void update_tick(void);

/**
 * @brief Returns the timer IRQ number attached to the scheduler.
 * 
 * @details Returns the timer IRQ number attached to the scheduler.
 *
 * @return The IRQ number of the timer that is attached to the scheduler.
 */
int32_t get_IRQ_SCHED_TIMER(void);

/**
 * @brief Returns the timer interrupt line attached to the scheduler.
 * 
 * @details Returns the timer interrupt line attached to the scheduler.
 *
 * @return The interrupt line of the timer that is attached to the scheduler.
 */
int32_t get_line_SCHED_HW(void);

/** 
 * @brief Returns the current uptime.
 * 
 * @details Return the current uptime of the system in seconds.
 *
 * @return The current uptime in seconds.
 */
uint32_t get_current_uptime(void);

/**
 * @brief Returns the number of system ticks since the system started.
 * 
 * @details Returns the number of system ticks since the system started.
 *
 * @returns The number of system ticks since the system started.
 */
uint32_t get_tick_count(void);

#endif /* __INTERRUPTS_H_ */