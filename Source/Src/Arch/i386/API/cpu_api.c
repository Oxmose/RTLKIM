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

#include <Lib/stdint.h>          /* Generic int types */
#include <BSP/lapic.h>           /* lapic_get_id() */
#include <Core/thread.h>         /* kernel_thread_t */

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
    /* If lapic is not activated but we only use one CPU */
    if(MAX_CPU_COUNT == 1)
    {
        return 0;
    }    
    return lapic_get_id();
}

void cpu_init_thread_context(void (*entry_point)(void), 
                             const uint32_t stack_index, 
                             const uint32_t free_table_page,
                             const uint32_t page_table_address,
                             kernel_thread_t* thread)
{
    /* Set EIP, ESP and EBP */
    thread->cpu_context.eip = (uint32_t)entry_point;
    thread->cpu_context.esp = (uint32_t)&thread->stack[stack_index - 17];
    thread->cpu_context.ebp = (uint32_t)&thread->stack[stack_index - 1];

    /* Set CR3 and free page table */
    thread->cpu_context.cr3 = page_table_address;
    thread->free_page_table = free_table_page;    

    /* Init thread stack */
    thread->stack[stack_index - 1]  = THREAD_INIT_EFLAGS;
    thread->stack[stack_index - 2]  = THREAD_INIT_CS;
    thread->stack[stack_index - 3]  = thread->cpu_context.eip;
    thread->stack[stack_index - 4]  = 0; /* UNUSED (error) */
    thread->stack[stack_index - 5]  = 0; /* UNUSED (int id) */
    thread->stack[stack_index - 6]  = THREAD_INIT_DS;
    thread->stack[stack_index - 7]  = THREAD_INIT_ES;
    thread->stack[stack_index - 8]  = THREAD_INIT_FS;
    thread->stack[stack_index - 9]  = THREAD_INIT_GS;
    thread->stack[stack_index - 10] = THREAD_INIT_SS;
    thread->stack[stack_index - 11] = THREAD_INIT_EAX;
    thread->stack[stack_index - 12] = THREAD_INIT_EBX;
    thread->stack[stack_index - 13] = THREAD_INIT_ECX;
    thread->stack[stack_index - 14] = THREAD_INIT_EDX;
    thread->stack[stack_index - 15] = THREAD_INIT_ESI;
    thread->stack[stack_index - 16] = THREAD_INIT_EDI;
    thread->stack[stack_index - 17] = thread->cpu_context.ebp;
    thread->stack[stack_index - 18] = thread->cpu_context.esp;
}