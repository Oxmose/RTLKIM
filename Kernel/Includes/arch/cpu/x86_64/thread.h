/*******************************************************************************
 * @file thread.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 03/10/2017
 *
 * @version 2.0
 *
 * @brief Thread's structures definitions.
 *
 * @details Thread's structures definitions. The files contains all the data
 * relative to the thread's management in the system (thread structure, thread
 * state).
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __X86_64_THREAD_H_
#define __X86_64_THREAD_H_

#include <lib/stdint.h>        /* Generic int types */
#include <cpu_settings.h>      /* CPU settings */

/* UTK configuration file */
#include <config.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Thread's initial EFLAGS register value. */
#define THREAD_INIT_RFLAGS 0x202 /* INT | PARITY */
/** @brief Thread's initial EAX register value. */
#define THREAD_INIT_RAX    0
/** @brief Thread's initial EBX register value. */
#define THREAD_INIT_RBX    0
/** @brief Thread's initial ECX register value. */
#define THREAD_INIT_RCX    0
/** @brief Thread's initial EDX register value. */
#define THREAD_INIT_RDX    0
/** @brief Thread's initial ESI register value. */
#define THREAD_INIT_RSI    0
/** @brief Thread's initial EDI register value. */
#define THREAD_INIT_RDI    0
/** @brief Thread's initial R8 register value. */
#define THREAD_INIT_R8     0
/** @brief Thread's initial R9 register value. */
#define THREAD_INIT_R9     0
/** @brief Thread's initial R10 register value. */
#define THREAD_INIT_R10    0
/** @brief Thread's initial R11 register value. */
#define THREAD_INIT_R11    0
/** @brief Thread's initial R12 register value. */
#define THREAD_INIT_R12    0
/** @brief Thread's initial R13 register value. */
#define THREAD_INIT_R13    0
/** @brief Thread's initial R14 register value. */
#define THREAD_INIT_R14    0
/** @brief Thread's initial R15 register value. */
#define THREAD_INIT_R15    0
/** @brief Thread's initial CS register value. */
#define THREAD_INIT_CS     THREAD_KERNEL_CS
/** @brief Thread's initial SS register value. */
#define THREAD_INIT_SS     THREAD_KERNEL_DS
/** @brief Thread's initial DS register value. */
#define THREAD_INIT_DS     THREAD_KERNEL_DS
/** @brief Thread's initial ES register value. */
#define THREAD_INIT_ES     THREAD_KERNEL_DS
/** @brief Thread's initial FS register value. */
#define THREAD_INIT_FS     THREAD_KERNEL_DS
/** @brief Thread's initial GS register value. */
#define THREAD_INIT_GS     THREAD_KERNEL_DS

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

#endif /* #ifndef __X86_64_THREAD_H_ */