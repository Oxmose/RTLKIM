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

#include <Cpu/cpu.h>              /* cpu_test_and_set() */
#include <Cpu/smp.h>              /* smp_get_booted_cpu_count() */
#include <API/cpu_api.h>          /* cpu_get_id() */
#include <Lib/stdint.h>           /* Generic int types */
#include <Interrupt/interrupts.h> /* kernel_interrupt_disable(),
                                   * kernel_interrupt_restore() */

/* UTK configuration file */
#include <config.h>

/**
 * @brief Use the pause method spinlock.
 *
 * @detail Wait on the spinlock given as parameter using the pause method to
 * allow fairness.
 *
 * @param uint32_t The spinlock to use.
 */
extern void pause_spinlock(volatile uint32_t* lockword);

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

#if MAX_CPU_COUNT > 1
/**
 * @brief Defines the spinlock structure.
 *
 */
struct spinlock
{
    /**
     * @brief Current lock value.
     *
     */
    volatile uint32_t value;
    /**
     * @brief Current owner thread's ID.
     *
     */
    volatile int32_t current_tid;
    /**
     * @brief Nesting count.
     *
     */
    volatile uint32_t nesting;
};
typedef volatile struct spinlock spinlock_t;
#endif

#if MAX_CPU_COUNT <= 1
/**
 * @brief Enters a critical section in the kernel.
 *
 * @details Enters a critical section in the kernel. Save interrupt state and
 * disables interrupts.
 */
#define ENTER_CRITICAL(x) {         \
    x = kernel_interrupt_disable(); \
}
#else
/**
 * @brief Enters a critical section in the kernel.
 *
 * @details Enters a critical section in the kernel. Save interrupt state and
 * disables interrupts.
 */
#define ENTER_CRITICAL(x, lock) {                         \
    x = kernel_interrupt_disable();                       \
    if(smp_get_booted_cpu_count() > 1)                    \
    {                                                     \
        int32_t cpu_id = cpu_get_id();                  \
        if(cpu_id != -1 && (lock)->current_tid != cpu_id) \
        {                                                 \
            pause_spinlock(&(lock)->value);               \
        }                                                 \
        (lock)->current_tid = cpu_id;                     \
        (lock)->nesting++;                                \
    }                                                     \
}
#endif

#if MAX_CPU_COUNT <= 1
/**
 * @brief Exits a critical section in the kernel.
 *
 * @details Exits a critical section in the kernel. Restore the previous
 * interrupt state.
 */
#define EXIT_CRITICAL(x) {           \
    kernel_interrupt_restore(x);     \
}
#else
/**
 * @brief Exits a critical section in the kernel.
 *
 * @details Exits a critical section in the kernel. Restore the previous
 * interrupt state.
 */
#define EXIT_CRITICAL(x, lock) {         \
    if(smp_get_booted_cpu_count() > 1) { \
        (lock)->nesting--;               \
        if((lock)->nesting == 0){        \
            (lock)->current_tid = -1;    \
        }                                \
        (lock)->value = 0;               \
    }                                    \
    kernel_interrupt_restore(x);         \
}
#endif

#if MAX_CPU_COUNT > 1
/**
 * @brief Initialize a spinlock.
 *
 * @details Initialize the spinlock to the start value.
 */
#define INIT_SPINLOCK(lock) {  \
    (lock)->value       = 0;   \
    (lock)->current_tid = -1;  \
    (lock)->nesting     = 0;   \
}

#define SPINLOCK_INIT_VALUE {0, -1, 0}
#endif
/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

#endif /* __CRITICAL_H_ */