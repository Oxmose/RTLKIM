/***************************************************************************//**
 * @file exceptions.h
 * 
 * @see exceptions.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 27/05/2018
 *
 * @version 2.0
 *
 * @brief Exceptions manager. 
 * 
 * @warning These functions must be called during or after the interrupts are 
 * set.
 * 
 * @details Exception manager. Allows to attach ISR to exceptions lines.
 * 
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __EXCEPTIONS_H_
#define __EXCEPTIONS_H_

#include <Lib/stdint.h>           /* Generic int types */
#include <Lib/stddef.h>           /* OS_RETURN_E */
#include <Cpu/cpu_settings.h> /* cpu_state_t, stack_state_t */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Minimal customizable accepted exception line. */
#define MIN_EXCEPTION_LINE 0
/** @brief Maximal customizable accepted exception line. */
#define MAX_EXCEPTION_LINE 31

/** @brief Divide by zero exception line. */
#define DIV_BY_ZERO_LINE 0

/** @brief Device not found exception. */
#define DEVICE_NOT_FOUND_LINE 7

/** @brief Page fault exception */
#define PAGE_FAULT_LINE 14

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initializes the exception manager.
 * 
 * @details Blanks the handlers memory and initialize the first 32 exceptions to
 * catch intel exceptions.
 * 
 * @return The success state or the error code. 
 * - OS_NO_ERR is returned if no error is encountered. 
 */
OS_RETURN_E kernel_exception_init(void);

/**
 * @brief Registers a new exception handler for the desired exception line.
 * 
 * @details Registers a custom exception handler to be executed. The exception 
 * line must be greater or equal to the minimal authorized custom exception line 
 * and less than the maximal one.
 *
 * @param[in] exception_line The exception line to attach the handler to.
 * @param[in] handler The handler for the desired exception.
 * 
 * @return The success state or the error code. 
 * - OS_NO_ERR is returned if no error is encountered. 
 * - OR_ERR_UNAUTHORIZED_INTERRUPT_LINE is returned if the desired
 * exception line is not allowed. 
 * - OS_ERR_NULL_POINTER is returned if the pointer
 * to the handler is NULL. 
 * - OS_ERR_INTERRUPT_ALREADY_REGISTERED is returned if a 
 * handler is already registered for this exception line.
 */
OS_RETURN_E kernel_exception_register_handler(const uint32_t exception_line,
                                       void(*handler)(
                                             cpu_state_t*,
                                             address_t,
                                             stack_state_t*
                                             )
                                       );

/**
 * @brief Unregisters a new exception handler for the desired exception line.
 * 
 * @details Unregisters a custom exception handler to be executed. The exception 
 * line must be greater or equal to the minimal authorized custom exception line 
 * and less than the maximal one.
 *
 * @param[in] exception_line The exception line to deattach the handler from.
 * 
 * @return The success state or the error code. 
 * - OS_NO_ERR is returned if no error is encountered. 
 * - OR_ERR_UNAUTHORIZED_INTERRUPT_LINE is returned if the desired
 * exception line is not allowed.
 * - OS_ERR_INTERRUPT_NOT_REGISTERED is returned if the exception line has no
 * handler attached.
 */
OS_RETURN_E kernel_exception_remove_handler(const uint32_t exception_line);

#endif /* __EXCEPTIONS_H_ */