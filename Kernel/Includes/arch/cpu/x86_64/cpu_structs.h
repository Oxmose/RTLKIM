/*******************************************************************************
 * @file cpu_structs.h
 *
 * @see cpu_structs.c
 * 
 * @author Alexy Torres Aurora Dugo
 *
 * @date 14/12/2017
 *
 * @version 1.0
 *
 * @brief x86_64 CPU structures.
 *
 * @details x86_64 CPU structures. IDT, GDT and CPU stacks are defined here
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __X86_64_CPU_STRUCTS_H_
#define __X86_64_CPU_STRUCTS_H_

#include <lib/stddef.h> /* Standard definitions */

/* UTK configuration file */
#include <config.h>

/*******************************************************************************
 * DEFINITIONS
 ******************************************************************************/

/** @brief Number of entries in the kernel's GDT. */
#define GDT_ENTRY_COUNT (8 + MAX_CPU_COUNT)

/** @brief Number of entries in the kernel's IDT. */
#define IDT_ENTRY_COUNT 256

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/** 
 * @brief Define the GDT pointer, contains the  address and limit of the GDT.
 */
struct gdt_ptr
{
    /** @brief The GDT size. */
    uint16_t size;

    /** @brief The GDT address. */
    uintptr_t base;
}__attribute__((packed));

/** 
 * @brief Defines the gdt_ptr_t type as a shortcut for struct  gdt_ptr. 
 */
typedef struct gdt_ptr gdt_ptr_t;

/** 
 * @brief Define the IDT pointer, contains the  address and limit of the IDT.
 */
struct idt_ptr
{
    /** @brief The IDT size. */
    uint16_t size;

    /** @brief The IDT address. */
    uintptr_t base;
}__attribute__((packed));

/** 
 * @brief Defines the idt_ptr_t type as a shortcut for struct idt_ptr. 
 */
typedef struct idt_ptr idt_ptr_t;

/** @brief Holds the CPU register values */
struct cpu_state
{
    /** @brief CPU's esp register. */
    uint64_t rsp;
    /** @brief CPU's ebp register. */
    uint64_t rbp;
    /** @brief CPU's edi register. */
    uint64_t rdi;
    /** @brief CPU's esi register. */
    uint64_t rsi;
    /** @brief CPU's edx register. */
    uint64_t rdx;
    /** @brief CPU's ecx register. */
    uint64_t rcx;
    /** @brief CPU's ebx register. */
    uint64_t rbx;
    /** @brief CPU's eax register. */
    uint64_t rax;

    /** @brief CPU's r15 register. */
    uint64_t r15;
    /** @brief CPU's r14 register. */
    uint64_t r14;
    /** @brief CPU's r13 register. */
    uint64_t r13;
    /** @brief CPU's r12 register. */
    uint64_t r12;
    /** @brief CPU's r11 register. */
    uint64_t r11;
    /** @brief CPU's r10 register. */
    uint64_t r10;
    /** @brief CPU's r9 register. */
    uint64_t r9;
    /** @brief CPU's r8 register. */
    uint64_t r8;

    /** @brief CPU's ss register. */
    uint64_t ss;
    /** @brief CPU's gs register. */
    uint64_t gs;
    /** @brief CPU's fs register. */
    uint64_t fs;
    /** @brief CPU's es register. */
    uint64_t es;
    /** @brief CPU's ds register. */
    uint64_t ds;
} __attribute__((packed));

/** 
 * @brief Defines cpu_state_t type as a shorcut for struct cpu_state.
 */
typedef struct cpu_state cpu_state_t;

/** @brief Hold the stack state before the interrupt */
struct stack_state
{
    /** @brief Interrupt ID */
    uint64_t int_id;
    /** @brief Interrupt's error code. */
    uint64_t error_code;
    /** @brief RIP of the faulting instruction. */
    uint64_t rip;
    /** @brief CS before the interrupt. */
    uint64_t cs;
    /** @brief RFLAGS before the interrupt. */
    uint64_t rflags;
} __attribute__((packed));

/** 
 * @brief Defines stack_state_t type as a shorcut for struct stack_state.
 */
typedef struct stack_state stack_state_t;

/**  
 * @brief CPU TSS abstraction structure. This is the representation the kernel 
 * has of an intel's TSS entry.
 */
struct cpu_tss_entry 
{
    uint32_t reserved0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t reserved2;
    uint16_t iomap_base;
    uint16_t reserved3;
} __attribute__((__packed__));

/** 
 * @brief Defines the cpu_tss_entry_t type as a shortcut for struct 
 * cpu_tss_entry. 
 */
typedef struct cpu_tss_entry cpu_tss_entry_t;

/**
 * @brief Defines he virtual CPU context for the CPU.
 */
struct virtual_cpu_context
{
    /** @brief Thread's specific RSP registers. */
    uint64_t rsp;
    /** @brief Thread's specific RBP registers. */
    uint64_t rbp;
    /** @brief Thread's specific RIP registers. */
    uint64_t rip;

     /** @brief Thread's CR3 page directory pointer. */
    uint64_t cr3;    
};

/** @brief Shortcut name for the struct virtual_cpu_context structure. */
typedef struct virtual_cpu_context virtual_cpu_context_t;

/** 
 * @brief CPU IDT entry. Describes an entry in the IDT.
 */
struct cpu_idt_entry
{
    /** @brief ISR low address. */
    uint16_t off_low;

    /** @brief Code segment selector. */
    uint16_t c_sel;

    /** @brief Entry IST number. */
    uint8_t ist;

    /** @brief Entry flags. */
    uint8_t flags;

    /** @brief ISR middle address. */
    uint16_t off_mid;

    /** @brief ISR high address. */
    uint32_t off_hig;

    /** @brief Must be zero. */
    uint32_t reserved1;
};

/** 
 * @brief Defines the cpu_idt_entry_t type as a shortcut for struct 
 * cpu_idt_entry. 
 */
typedef struct cpu_idt_entry cpu_idt_entry_t;

struct kernel_thread;
typedef struct kernel_thread kernel_thread_t;

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/** @brief CPU GDT space in memory. */
extern uint64_t cpu_gdt[GDT_ENTRY_COUNT];
/** @brief Kernel GDT structure */
extern gdt_ptr_t cpu_gdt_ptr;

/** @brief CPU IDT space in memory. */
extern cpu_idt_entry_t cpu_idt[IDT_ENTRY_COUNT];
/** @brief Kernel IDT structure */
extern idt_ptr_t cpu_idt_ptr;

/** @brief CPU TSS structures */
extern cpu_tss_entry_t cpu_tss[MAX_CPU_COUNT];

/** @brief Kernel stacks */
extern uint8_t cpu_stacks[MAX_CPU_COUNT][KERNEL_STACK_SIZE];

#endif /* #ifndef __X86_64_CPU_STRUCTS_H_ */