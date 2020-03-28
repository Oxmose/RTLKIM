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

#include <Lib/stddef.h>           /* address_t */
#include <Lib/stdint.h>           /* Generic int types */
#include <Interrupt/interrupts.h> /* cpu_state, stack_state */

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
                             const address_t stack_index, 
                             const address_t free_table_page,
                             const address_t page_table_address,
                             kernel_thread_t* thread);

/** 
 * @brief Returns the current page directory physical address.
 * 
 * @details Return the current value stored in CR3. No error can be returned.
 * 
 * @return The current CR3 value.
 */
address_t cpu_get_current_pgdir(void);

/**
 * @brief Saves the current thread CPU context.
 * 
 * @details Saves the current CPU context for the thread.
 * Registers (and other data) should be saved here.
 * 
 * @param[in] first_sche Should be 0 if the CPU has never been scheduled
 * before. Otherwise this value should be 1.
 * @param[in] cpu_state The current CPU state.
 * @param[in] stack_stack The current thread's stack state.
 * @param[out] thread The thread structure to save the data to.
 */
void cpu_save_context(const uint32_t first_sched,
                      const cpu_state_t* cpu_state, 
                      const stack_state_t* stack_state, 
                      kernel_thread_t* thread);

/**
 * @brief Update the CPU's page directory.
 *
 * @details Update the current CPU CR3 with the new page directory
 * physical address.
 * 
 * @param[in] new_pgdir The physical address of the new page directory.
 */
void cpu_update_pgdir(const address_t new_pgdir);

/**
 * @brief Restores the thread's CPU context.
 * 
 * @details Restores the thread's CPU context from the thread storage 
 * structure. Registers are updated and the execution flow might be
 * updated.
 * 
 * @param[out] cpu_state The current CPU state that will be modified.
 * @param[in] stack_state The current stack state.
 * @param[in] thread The thread structure to read the data from.
 */
void cpu_restore_context(cpu_state_t* cpu_state, 
                         const stack_state_t* stack_state, 
                         const kernel_thread_t* thread);

/**
 * @brief Sets the next thread's isntruction.
 * 
 * @details Modifies the thread's next instruction pointer to 
 * execute a different execution flow.
 * 
 * @param[in] cpu_state The current CPU state.
 * @param[out] stack_state The current stack state that will be modified.
 * @param[in] next_inst The address of the next instruction to be executed by
 * the thread.
 */
void cpu_set_next_thread_instruction(const cpu_state_t* cpu_state,
                                     stack_state_t* stack_state, 
                                     const address_t next_inst);

/**
 * @brief Raises CPU interrupt.
 * 
 * @details Raises a software CPU interrupt on the desired line.
 * 
 * @param[in] interrupt_line The line on which the interrupt should be raised.
 * 
 * @return OS_NO_ERR shoudl be return in case of success.
 * - OS_ERR_UNAUTHORIZED_ACTION Is returned if the interrupt line is not
 * correct.
 * - Other errors value may be returned depending on the underlying function
 * calls made by this function.
 */
OS_RETURN_E cpu_raise_interrupt(const uint32_t interrupt_line);

/**
 * @brief Returns the CPU current interrupt state.
 * 
 * @details Returns the current CPU eflags interrupt enable value.
 * 
 * @return The CPU current interrupt state: 1 if enabled, 0 otherwise.
 */
uint32_t cpu_get_interrupt_state(void);

/** 
 * @brief Returns the saved interrupt state.
 * 
 * @details Returns the saved interrupt state based on the stack state.
 * 
 * @param[in] cpu_state The current CPU state.
 * @param[in] stack_state The current stack state.
 * 
 * @return The current savec interrupt state: 1 if enabled, 0 otherwise.
 */
uint32_t cpu_get_saved_interrupt_state(const cpu_state_t* cpu_state,
                                       const stack_state_t* stack_state);


#endif /* __CPU_API_H_ */