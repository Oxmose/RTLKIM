/***************************************************************************//**
 * @file cpu_api.c
 *
 * @see cpu_api.h
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

#include <Lib/stdint.h>       /* Generic int types */
#include <Cpu/gic.h>          /* gic_get_status */
#include <Cpu/cpu_settings.h> /* CPU structures. */    

/* RTLK configuration file */
#include <config.h>

/* Header file */
#include <API/cpu_api.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/* None */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

int32_t cpu_get_id(void)
{
    uint32_t cpu_id;

    /* Read the CPU affinity register */
    __asm__ __volatile__("MRC p15, 0, r1, c0, c0, 5\n\tstr r1, %0"
        : "=m" (cpu_id)
        : /* no input */
        : "r1");

    /* The CPU id is the cluster ID and CPU id mixed */
    cpu_id = ((cpu_id >> 8) & 0x3C) | (cpu_id & 0x3);
    return cpu_id;
}

uint32_t cpu_get_interrupt_state(void)
{
    return gic_get_status();
}

uint32_t cpu_get_saved_interrupt_state(const cpu_state_t* cpu_state,
                                       const stack_state_t* stack_state)
{
    (void) cpu_state;
    (void) stack_state;
    /* TODO */
    return 1;
}

void cpu_set_next_thread_instruction(const cpu_state_t* cpu_state,
                                     stack_state_t* stack_state, 
                                     const uint32_t next_inst)
{
    (void) cpu_state;
    (void) stack_state;
    (void) next_inst;
    /* Todo */
}