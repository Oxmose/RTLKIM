/***************************************************************************//**
 * @file critical.h
 * 
 * @see critical.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 08/10/2018
 *
 * @version 1.0
 *
 * @brief Kernel's concurency management module.
 * 
 * @details Kernel's concurency management module. Defines the different basic 
 * synchronization primitives used in the kernel.
 * 
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __CRITICAL_H_
#define __CRITICAL_H_

#include <Lib/stdint.h>           /* Generic int types */
#include <Interrupt/interrupts.h> /* kernel_interrupt_disable(), 
                                   * kernel_interrupt_restore() */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** 
 * @brief Enters a critical section in the kernel.
 * 
 * @details Enters a critical section in the kernel. Save interrupt state and
 * disables interrupts.
 */
#define ENTER_CRITICAL(x) {                           \
    x = kernel_interrupt_disable(); \
}

/** 
 * @brief Exits a critical section in the kernel.
 * 
 * @details Exits a critical section in the kernel. Restore the previous 
 * interrupt state.
 */
#define EXIT_CRITICAL(x) {                            \
    kernel_interrupt_restore(x);             \
}

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

#endif /* __CRITICAL_H_ */