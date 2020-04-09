/***************************************************************************//**
 * @file cpu_settings.h
 * 
 * @see cpu_settings.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 14/12/2017
 *
 * @version 1.0
 *
 * @brief ARMV7 CPU abstraction functions and definitions. 
 * 
 * @details ARMV7 CPU abstraction: setting functions and structures.d
 * 
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __CPU_SETTINGS_H_
#define __CPU_SETTINGS_H_

#include <Lib/stdint.h> /* Generic int types */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/***************************
 * INT/IRQ Settings
 **************************/

/** @brief Defines the kernel's maximal entry count. */
#define INT_ENTRY_COUNT 255

/** @brief Offset of the first line of an IRQ interrupt from PIC. */
#define INT_PIC_IRQ_OFFSET     0x30
/** @brief Offset of the first line of an IRQ interrupt from IO-APIC. */
#define INT_IOAPIC_IRQ_OFFSET  0x40
/** @brief Minimal customizable accepted interrupt line. */
#define MIN_INTERRUPT_LINE     0x20
/** @brief Maximal customizable accepted interrupt line. */
#define MAX_INTERRUPT_LINE     0xFF

/** @brief Scheduler software interrupt line. */
#define SCHEDULER_SW_INT_LINE      0x21
/** @brief Panic software interrupt line. */
#define PANIC_INT_LINE             0x2A

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/** @brief Holds the CPU register values */
struct cpu_state
{
    /** @brief CPU's SP register. */
    uint32_t sp;
    /** @brief CPU's LR register. */
    uint32_t lr;
    /** @brief CPU's PC register. */
    uint32_t pc;

    /** @brief CPU's registers. */
    uint32_t registers[13];

} __attribute__((packed));

/** 
 * @brief Defines cpu_state_t type as a shorcut for struct cpu_state.
 */
typedef struct cpu_state cpu_state_t;

/** @brief Hold the stack state before the interrupt */
struct stack_state
{
    /** @brief CSPR value before interrupt. */
    uint32_t cspr;
} __attribute__((packed));

/** 
 * @brief Defines stack_state_t type as a shorcut for struct stack_state.
 */
typedef struct stack_state stack_state_t;


/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

#endif /* __CPU_SETTINGS_H_ */
