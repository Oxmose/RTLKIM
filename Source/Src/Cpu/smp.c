/*******************************************************************************
 *
 * File: smp.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 27/03/2018
 *
 * Version: 1.0
 *
 * SMP implementation of the kernel. The different functions in this File
 * allow the systen to detect, initialize and manage CPU cores.
 ******************************************************************************/

#include <Lib/stdint.h>            /* Generic int types */
#include <Lib/stddef.h>            /* OS_RETURN_E */
#include <BSP/acpi.h>              /* acpi_get_detected_cpu_count */
#include <BSP/lapic.h>             /* get_lapic_id */
#include <BSP/pit.h>               /* pit_disable() */
#include <Interrupt/interrupts.h>  /* interrupts structures (cpu_state_t...) */
#include <Interrupt/panic.h>       /* kernel_panic */
#include <IO/kernel_output.h>      /* kernel_info */
#include <Core/scheduler.h>        /* init_ap_scheduler */
#include <Cpu/cpu.h>               /* hlt */
#include <Memory/paging.h>         /* kernel_mmap */

/* RTLK configuration file */
#include <config.h>

/* Header file */
#include <Cpu/smp.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/
static int32_t  cpu_count;
static uint32_t main_core_id;

static const uint32_t* cpu_ids;
static const local_apic_t** cpu_lapics;

extern volatile uint8_t init_cpu_count;
static volatile uint8_t init_seq_end;

/* Kernel IDT structure */
extern uint64_t cpu_idt[IDT_ENTRY_COUNT];
extern uint16_t cpu_idt_size;
extern uint32_t cpu_idt_base;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Init entry point for AP */
extern void ap_boot_loader_init(void);

OS_RETURN_E smp_init(void)
{
    uint32_t i;
    OS_RETURN_E err;

    /* Get the number of core of the system */
    cpu_count = acpi_get_detected_cpu_count();

    /* One core detected, nothing to do */
    if(cpu_count <= 1)
    {
        return OS_NO_ERR;
    }

    init_seq_end = 0;

    kernel_info("Init %d CPU cores\n", cpu_count);

    main_core_id = lapic_get_id();

    kernel_info("Main core ID %d\n", main_core_id);

    /* Get LAPIC ids */
    cpu_ids    = acpi_get_cpu_ids();
    cpu_lapics = acpi_get_cpu_lapics();

    /* Map needed memory */
    err = kernel_direct_mmap((void*)0x4000, (void*)0x4000, 0x1,
                             PG_DIR_FLAG_PAGE_SIZE_4KB |
                             PG_DIR_FLAG_PAGE_SUPER_ACCESS |
                             PG_DIR_FLAG_PAGE_READ_WRITE,
                             1);

    if(OS_NO_ERR != err)
    {
        return err;
    }

    /* Init startup code in low mem */
    ap_boot_loader_init();

    /* Wait for CPU to INIT */
    init_cpu_count = 1;

    /* Init each sleeping core */
    for(i = 0; i < (uint32_t)cpu_count; ++i)
    {
        uint32_t current_cpu_init;

        current_cpu_init = init_cpu_count;
        if(cpu_ids[i] == main_core_id) continue;

        err = lapic_send_ipi_init(cpu_lapics[i]->apic_id);
        if(err != OS_NO_ERR)
        {
            kernel_error("Cannot send INIT IPI [%d]\n", err);
            kernel_panic(err);
        }

        kernel_interrupt_restore(1);
        time_wait_no_sched(10);
        kernel_interrupt_disable();

        /* Send startup */
        err = lapic_send_ipi_startup(cpu_lapics[i]->apic_id, 0x4);
        if(err != OS_NO_ERR)
        {
            kernel_error("Cannot send STARTUP IPI [%d]\n", err);
            kernel_panic(err);
        }

        kernel_interrupt_restore(1);
        time_wait_no_sched(30);
        kernel_interrupt_disable();

        if(current_cpu_init == init_cpu_count)
        {
            /* Send startup */
            err = lapic_send_ipi_startup(cpu_lapics[i]->apic_id, 0x4);
            if(err != OS_NO_ERR)
            {
                kernel_error("Cannot send STARTUP IPI [%d]\n", err);
                kernel_panic(err);
            }
        }

        /* Wait for the current AP to Init */
        while(current_cpu_init == init_cpu_count);
    }

    init_seq_end = 1;

    /* Make sure all the AP are initialized, we should never block here */
    while(init_cpu_count < cpu_count);

    return OS_NO_ERR;
}

void smp_ap_core_init(void)
{
    OS_RETURN_E err;
    uint32_t cpu_id = lapic_get_id();



    /* Load AP TSS */
    __asm__ __volatile__("ltr %0" : : "rm" ((uint16_t)
                                            (TSS_SEGMENT + cpu_id * 0x08)));



    /* Init local APIC */
    err = lapic_init();
    if(err != OS_NO_ERR)
    {
        kernel_error("Local APIC Initialization error %d [CPU %d]\n", err,
                     cpu_id);
        kernel_panic(err);
    }

#if 0
    /* Init LAPIC TIMER */
    err = init_lapic_ap_timer();
    if(err == OS_NO_ERR)
    {
        kernel_success("Local APIC TIMER Initialized [CPU %d]\n", cpu_id);
    }
    else
    {
        kernel_error("Local APIC TIMER Initialization error %d [CPU %d]\n",
                     err, cpu_id);
        kernel_panic(err);
    }

    kernel_info("Core %d initialized\n", cpu_id);

    /* Init Scheduler */
    err = init_ap_scheduler();

    kernel_error("End of kernel reached by AP Core %d [%d]\n", cpu_id, err);
    kernel_panic(err);
#endif

    kernel_info("CPU %d booted, idling...\n", cpu_id);

    ++init_cpu_count;

    while(init_seq_end == 0);

    while(1)
    {
        cpu_hlt();
    }
}