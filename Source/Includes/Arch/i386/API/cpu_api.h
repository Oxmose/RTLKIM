/***************************************************************************//**
 * @file cpu_api.h
 *
 * @see cpu_api.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 02/05/2019
 *
 * @version 1.0
 *
 * @brief X86 CPU API functions.
 *
 * @details X86 CPU API. Gathers generic API calls to manipulate the CPU
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __CPU_API_H_
#define __CPU_API_H_

#include <Lib/stdint.h>          /* Generic int types */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

struct virtual_cpu_context
{
    /** @brief Thread's specific ESP registers. */
    uint32_t esp;
    /** @brief Thread's specific EBP registers. */
    uint32_t ebp;
    /** @brief Thread's specific EIP registers. */
    uint32_t eip;

     /** @brief Thread's CR3 page directory pointer. */
    uint32_t cr3;    
};

typedef struct virtual_cpu_context virtual_cpu_context_t;

struct kernel_thread;
typedef struct kernel_thread kernel_thread_t;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Returns the current CPU id.
 * 
 * @details The function returns the CPU id on which the call is made.
 *
 * @return The current CPU id is returned on succes and -1 is return in case 
 * of error.
 */
int32_t cpu_get_id(void);

/**
 * @brief Initializes the thread's context.
 * 
 * @details Initializes the thread's context by populating the virtual CPU
 * structure of the thread and its stack.
 * 
 * @param[in] entry_point The thread's entry point.
 * @param[in] stack_index The thread's stack start index (the last element
 * of the  stack).
 * @param[in] free_table_page The free table page address for the new thread.
 * @param[in] page_table_address The page table physical address for the new
 * thread.
 * @param[out] thread The thread to initialize.
 */
void cpu_init_thread_context(void (*entry_point)(void), 
                             const uint32_t stack_index, 
                             const uint32_t free_table_page,
                             const uint32_t page_table_address,
                             kernel_thread_t* thread);

#endif /* __CPU_API_H_ */