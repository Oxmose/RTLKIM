/***************************************************************************//**
 * @file lapic.h
 * 
 * @see lapic.c
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


#ifndef __LAPIC_H_
#define __LAPIC_H_

#include <Lib/stdint.h>           /* Generic int types */
#include <Lib/stddef.h>           /* OS_RETURN_E */
#include <Time/time_management.h> /* kernel_timer_t */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/
#define LAPIC_ID                        0x0020
#define LAPIC_VER                       0x0030
#define LAPIC_TPR                       0x0080
#define LAPIC_APR                       0x0090
#define LAPIC_PPR                       0x00A0
#define LAPIC_EOI                       0x00B0
#define LAPIC_RRD                       0x00C0
#define LAPIC_LDR                       0x00D0
#define LAPIC_DFR                       0x00E0
#define LAPIC_SVR                       0x00F0
#define LAPIC_ISR                       0x0100
#define LAPIC_TMR                       0x0180
#define LAPIC_IRR                       0x0200
#define LAPIC_ESR                       0x0280
#define LAPIC_ICRLO                     0x0300
#define LAPIC_ICRHI                     0x0310
#define LAPIC_TIMER                     0x0320
#define LAPIC_THERMAL                   0x0330
#define LAPIC_PERF                      0x0340
#define LAPIC_LINT0                     0x0350
#define LAPIC_LINT1                     0x0360
#define LAPIC_ERROR                     0x0370
#define LAPIC_TICR                      0x0380
#define LAPIC_TCCR                      0x0390
#define LAPIC_TDCR                      0x03E0

/* Delivery Mode */
#define ICR_FIXED                       0x00000000
#define ICR_LOWEST                      0x00000100
#define ICR_SMI                         0x00000200
#define ICR_NMI                         0x00000400
#define ICR_INIT                        0x00000500
#define ICR_STARTUP                     0x00000600

/* Destination Mode */
#define ICR_PHYSICAL                    0x00000000
#define ICR_LOGICAL                     0x00000800

/* Delivery Status */
#define ICR_IDLE                        0x00000000
#define ICR_SEND_PENDING                0x00001000

/* Level */
#define ICR_DEASSERT                    0x00000000
#define ICR_ASSERT                      0x00004000

/* Trigger Mode */
#define ICR_EDGE                        0x00000000
#define ICR_LEVEL                       0x00008000

/* Destination Shorthand */
#define ICR_NO_SHORTHAND                0x00000000
#define ICR_SELF                        0x00040000
#define ICR_ALL_INCLUDING_SELF          0x00080000
#define ICR_ALL_EXCLUDING_SELF          0x000C0000

/* Destination Field */
#define ICR_DESTINATION_SHIFT           24

#define LAPIC_TIMER_MODE_PERIODIC       0x20000
#define LAPIC_DIVIDER_16                0x3
#define LAPIC_INIT_FREQ                 100
#define APIC_LVT_INT_MASKED             0x10000

#define LAPIC_SPURIOUS_INT_LINE MAX_INTERRUPT_LINE

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/** @brief LAPIC Timer driver instance. */
extern kernel_timer_t lapic_timer_driver;


/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Init CPU Local APIC
 *
 * @return OS_NO_ERR on succes, an error otherwise.
 */
OS_RETURN_E lapic_init(void);

/* Init CPU Local APIC TIMER
 *
 * @return OS_NO_ERR on succes, an error otherwise.
 */
OS_RETURN_E lapic_timer_init(void);


/* Init AP CPU Local APIC TIMER
 *
 * @return OS_NO_ERR on succes, an error otherwise.
 */
OS_RETURN_E lapic_ap_timer_init(void);

/* Returns the current CPU Local APIC ID.
 *
 * @returns The current CPU Local APIC ID.
 */
uint32_t lapic_get_id(void);

/* Send an INIT IPI to the CPU deisgned by the Local APIC ID given as parameter.
 * The ID is checked before sending the IPI.
 *
 * @param lapic_id The Local APIC ID of the CPU to send the IPI to.
 * @return OS_NO_ERR on success, an error otherwise.
 */
OS_RETURN_E lapic_send_ipi_init(const uint32_t lapic_id);

/* Send an STARTUP IPI to the CPU deisgned by the Local APIC ID given as 
 * parameter. The ID is checked before sending the IPI.
 *
 * @param lapic_id The Local APIC ID of the CPU to send the IPI to.
 * @param vector The startup IPI vector.
 * @return OS_NO_ERR on success, an error otherwise.
 */
OS_RETURN_E lapic_send_ipi_startup(const uint32_t lapic_id,
                                   const uint32_t vector);

/* Send an IPI to the CPU deisgned by the Local APIC ID given as parameter.
 * The ID is checked before sending the IPI.
 *
 * @param lapic_id The Local APIC ID of the CPU to send the IPI to.
 * @return OS_NO_ERR on success, an error otherwise.
 */
OS_RETURN_E lapic_send_ipi(const uint32_t lapic_id);

/* Set END OF INTERRUPT for the current CPU Local APIC.
 *
 * @param interrupt_line The intrrupt line for which the EOI should be set.
 * @return OS_NO_ERR on success, an error otherwise.
 */
OS_RETURN_E lapic_set_int_eoi(const uint32_t interrupt_line);

/**
 * @brief Enables LAPIC Timer ticks.
 * 
 * @details Enables LAPIC Timer ticks by clearing the LAPIC Timer's IRQ mask.
 *
 * @return The succes state or the error code. 
 * - OS_NO_ERR is returned if no error is encountered. 
 * - OS_ERR_NO_SUCH_IRQ_LINE is returned if the IRQ number of the LAPIC Timer is 
 * not supported.
 */
OS_RETURN_E lapic_timer_enable(void);

/**
 * @brief Disables LAPIC Timer ticks.
 * 
 * @details Disables LAPIC Timer ticks by setting the LAPIC Timer's IRQ mask.
 *
 * @return The succes state or the error code. 
 * - OS_NO_ERR is returned if no error is encountered. 
 * - OS_ERR_NO_SUCH_IRQ_LINE is returned if the IRQ number of the LAPIC Timer is 
 * not supported.
 */
OS_RETURN_E lapic_timer_disable(void);

/** 
 * @brief Sets the LAPIC Timer's tick frequency.
 * 
 * @details Sets the LAPIC Timer's tick frequency. The value must be between 
 * 20Hz and 8000Hz.
 * 
 * @warning The value must be between 20Hz and 8000Hz
 *
 * @param[in] freq The new frequency to be set to the LAPIC Timer.
 * 
 * @return The succes state or the error code. 
 * - OS_NO_ERR is returned if no error is encountered. 
 * - OS_ERR_OUT_OF_BOUND is returned if the frequency is out of bounds.
 * - OS_ERR_NO_SUCH_IRQ_LINE is returned if the IRQ number of the LAPIC Timer is 
 * not supported.
 */
OS_RETURN_E lapic_timer_set_frequency(const uint32_t freq);

/**
 * @brief Returns the LAPIC Timer tick frequency in Hz.
 * 
 * @details Returns the LAPIC Timer tick frequency in Hz.
 * 
 * @return The LAPIC Timer tick frequency in Hz.
 */
uint32_t lapic_timer_get_frequency(void);

/**
 * @brief Sets the LAPIC Timer tick handler.
 *
 * @details Sets the LAPIC Timer tick handler. This function will be called at 
 * each LAPIC Timer tick received.
 * 
 * @param[in] handler The handler of the LAPIC Timer interrupt.
 * 
 * @return The succes state or the error code. 
 * - OS_NO_ERR is returned if no error is encountered. 
 * - OS_ERR_NULL_POINTER is returned if the handler is NULL.
  * - OR_ERR_UNAUTHORIZED_INTERRUPT_LINE is returned if the LAPIC Timer 
  * interrupt line is not allowed. 
 * - OS_ERR_NULL_POINTER is returned if the pointer to the handler is NULL. 
 * - OS_ERR_INTERRUPT_ALREADY_REGISTERED is returned if a handler is already 
 * registered for the LAPIC Timer.
 */
OS_RETURN_E lapic_timer_set_handler(void(*handler)(
                                 cpu_state_t*,
                                 uint32_t,
                                 stack_state_t*
                                 ));

/**
 * @brief Removes the LAPIC Timer tick handler.
 *
 * @details Removes the LAPIC Timer tick handler. 
 * 
 * @return The succes state or the error code. 
 * - OS_NO_ERR is returned if no error is encountered. 
 * - OR_ERR_UNAUTHORIZED_INTERRUPT_LINE is returned if the LAPIC Timer interrupt
 * line is not allowed. 
 * - OS_ERR_INTERRUPT_NOT_REGISTERED is returned if the LAPIC Timer line has no 
 * handler attached.
 */
OS_RETURN_E lapic_timer_remove_handler(void);

/**
 * @brief Returns the LAPIC Timer IRQ number.
 * 
 * @details Returns the LAPIC Timer IRQ number.
 * 
 * @return The LAPIC Timer IRQ number.
 */
uint32_t lapic_timer_get_irq(void);


#endif /* __LAPIC_H_ */