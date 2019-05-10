/***************************************************************************//**
 * @file cpu_api.h
 *
 * @see cpu_api.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 09/05/2019
 *
 * @version 1.0
 *
 * @brief ARMV7 CPU API functions.
 *
 * @details ARMV7 CPU API. Gathers generic API calls to manipulate the CPU
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __CPU_API_H_
#define __CPU_API_H_

#include <Lib/stdint.h>       /* Generic int types */
#include <Cpu/cpu_settings.h> /* CPU structures */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/


struct virtual_cpu_context
{
    /** @brief Thread's specific SP registers. */
    uint32_t sp;   
};

typedef struct virtual_cpu_context virtual_cpu_context_t;

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
                                     const uint32_t next_inst);


#endif /* __CPU_API_H_ */