/***************************************************************************//**
 * @file kernel_kickstart.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 15/12/2017
 *
 * @version 1.0
 *
 * @brief Kernel's main boot sequence.
 *
 * @warning At this point interrupts should be disabled.
 *
 * @details Kernel's booting sequence. Initializes the rest of the kernel after
 *  GDT, IDT and TSS initialization. Initializes the hardware and software
 * core of the kernel.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <Cpu/cpu.h>              /* cpu_detect() */
#include <Cpu/smp.h>              /* init_smp() */
#include <IO/kernel_output.h>     /* kernel_output() */
#include <BSP/pic.h>              /* pic_init(), pic_driver */
#include <BSP/io_apic.h>          /* io-apic_init(), io_apic_driver */
#include <BSP/lapic.h>            /* lapic_init() */
#include <BSP/pit.h>              /* pit_init() */
#include <BSP/rtc.h>              /* rtc_init() */
#include <BSP/acpi.h>             /* acpi_init() */
#include <Interrupt/interrupts.h> /* kernel_interrupt_init() */
#include <Interrupt/exceptions.h> /* kernel_exception_init() */
#include <Interrupt/panic.h>      /* kernel_panic() */
#include <Memory/meminfo.h>       /* memory_map_init() */
#include <Memory/paging.h>        /* paging_init() */
#include <Drivers/vga_text.h>     /* vga_text_driver */
#include <Drivers/vesa.h>         /* init_vesa(), vesa_text_vga_to_vesa() */
#include <Drivers/keyboard.h>     /* keyboard_init() */
#include <Drivers/ata_pio.h>      /* ata_pio_init() */
#include <Lib/string.h>           /* strlen() */
#include <Core/scheduler.h>       /* sched_init() */

/* RTLK configuration file */
#include <config.h>

#if TEST_MODE_ENABLED
#include <Tests/test_bank.h>
#endif

#define INIT_MSG(msg_success, msg_error, error, panic) {     \
    if(error != OS_NO_ERR)                                   \
    {                                                        \
        kernel_error(msg_error, error);                      \
        if(panic == 1)                                       \
        {                                                    \
            kernel_panic(error);                             \
        }                                                    \
    }                                                        \
    else if(strlen(msg_success) != 0)                        \
    {                                                        \
        kernel_success(msg_success);                         \
    }                                                        \
}

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/
/** @brief Used for test purposes, this has to be deleted in the final version
 */
extern int main(void);

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Main boot sequence, C kernel entry point.
 *
 * @details Main boot sequence, C kernel entry point. Initializes each basic
 * drivers for the kernel, then init the scheduler and start the system.
 *
 * @warning This function should never return. In case of return, the kernel
 * should be able to catch the return as an error.
 */
void kernel_kickstart(void)
{
    OS_RETURN_E err;

    /* Init VGA display if needed */
    #if DISPLAY_TYPE == DISPLAY_VGA
    graphic_set_selected_driver(&vga_text_driver);
    #endif 

    #if KERNEL_DEBUG == 1
    kernel_serial_debug("Kickstarting the kernel\n");
    #endif

    #if TEST_MODE_ENABLED
    loader_ok_test();
    idt_ok_test();
    gdt_ok_test();
    output_test();
    kheap_test();
    vga_text_test();
    #endif

    kernel_printf("\n==== Kickstarting RTLK ====\n");

    /* Detect CPU */
    #if KERNEL_DEBUG == 1
    kernel_serial_debug("Detecting CPU\n");
    #endif
    err = cpu_detect(1);
    INIT_MSG("", "Error while detecting CPU: %d. HALTING\n",err, 1);

    /* Detect memory */
    #if KERNEL_DEBUG == 1
    kernel_serial_debug("Detecting memory\n");
    #endif
    err = memory_map_init();
    INIT_MSG("", "Error while detecting memory: %d. HALTING\n",err, 1);

    /* Enable paging */
    #if KERNEL_DEBUG == 1
    kernel_serial_debug("Enabling paging\n");
    #endif
    err = paging_init();
    INIT_MSG("Paging enabled\n", "Error while enabling paging: %d. HALTING\n",err, 1);

    #if TEST_MODE_ENABLED == 1
    paging_alloc_test();
    #endif

    /* Init VESA */
    #if (DISPLAY_TYPE == DISPLAY_VESA && TEST_MODE_ENABLED == 0) || \
         (TEST_MODE_ENABLED == 1 && VESA_TEXT_TEST == 1)
    err = vesa_init();
    if(err == OS_NO_ERR)
    {
        kernel_success("VESA Initialized\n");
        #if TEST_MODE_ENABLED == 1
        vesa_text_test();
        #else
        err = vesa_text_vga_to_vesa();
        if(err != OS_NO_ERR)
        {
            kernel_error("VESA switch error [%d]\n", err);
        }
        #endif
    }
    else
    {
        kernel_error("VESA Initialization error [%d]\n", err);
    }
    #endif

    /* Initialize ACPI */
    #if KERNEL_DEBUG == 1
    kernel_serial_debug("Initializing ACPI\n");
    #endif
    err = acpi_init();
    INIT_MSG("ACPI Initialized\n",
            "Error while initializing ACPI: %d. HALTING\n",
            err, 1);

    /* Init PIC */
    #if KERNEL_DEBUG == 1
    kernel_serial_debug("Initializing the PIC driver\n");
    #endif
    err = pic_init();
    INIT_MSG("PIC Initialized\n",
             "Error while initializing PIC: %d. HALTING\n",err, 1);
    #if TEST_MODE_ENABLED
    pic_driver_test();
    #endif

    /* Init IO-APIC */
    #if ENABLE_IO_APIC !=  0
    #if KERNEL_DEBUG == 1
    kernel_serial_debug("Initializing the IO-APIC driver\n");
    #endif
    err = io_apic_init();
    INIT_MSG("IO-APIC Initialized\n",
             "Error while initializing IO-APIC: %d. HALTING\n",err, 1);
    #if TEST_MODE_ENABLED
    io_apic_driver_test();
    #endif

    /* Init LAPIC driver */
    #if KERNEL_DEBUG == 1
    kernel_serial_debug("Initializing LAPIC driver\n");
    #endif
    err = lapic_init();
    INIT_MSG("LAPIC Initialized\n",
             "Error while initializing LAPIC: %d. HALTING\n",
            err, 1);
    #if TEST_MODE_ENABLED
    lapic_driver_test();
    #endif
    #endif

    /* Init kernel's interrupt manager */
    #if KERNEL_DEBUG == 1
    kernel_serial_debug("Initializing the kernel interrupt manager\n");
    #endif
    #if ENABLE_IO_APIC !=  0
    err = kernel_interrupt_init(&io_apic_driver);
    err |= pic_disable();
    #else
    err = kernel_interrupt_init(&pic_driver);
    #endif
    INIT_MSG("Kernel Interrupt Manager Initialized\n",
             "Error while initializing Kernel Interrupt Manager: %d. HALTING\n",
            err, 1);
    #if TEST_MODE_ENABLED
    interrupt_ok_test();
    panic_test();
    #endif

    /* Init kernel's exception manager */
    #if KERNEL_DEBUG == 1
    kernel_serial_debug("Initializing the kernel exception manager\n");
    #endif
    err = kernel_exception_init();
    INIT_MSG("Kernel Exception Manager Initialized\n",
             "Error while initializing Kernel Exception Manager: %d. HALTING\n",
            err, 1);
    #if TEST_MODE_ENABLED
    exception_ok_test();
    #endif

    /* Init PIT driver */
    #if KERNEL_DEBUG == 1
    kernel_serial_debug("Initializing PIT driver\n");
    #endif
    err = pit_init();
    INIT_MSG("PIT Initialized\n",
             "Error while initializing PIT: %d. HALTING\n",
            err, 1);
    #if TEST_MODE_ENABLED
    pit_driver_test();
    #endif

    /* Init RTC driver */
    #if KERNEL_DEBUG == 1
    kernel_serial_debug("Initializing RTC driver\n");
    #endif
    err = rtc_init();
    INIT_MSG("RTC Initialized\n",
             "Error while initializing RTC: %d. HALTING\n",
            err, 1);
    #if TEST_MODE_ENABLED
    rtc_driver_test();
    #endif

    /* Init LAPIC timer driver */
    #if ENABLE_IO_APIC != 0 && ENABLE_LAPIC_TIMER != 0
    #if KERNEL_DEBUG == 1
    kernel_serial_debug("Initializing LAPIC Timer driver\n");
    #endif
    err = lapic_timer_init();
    INIT_MSG("LAPIC Timer Initialized\n",
             "Error while initializing LAPIC Timer: %d. HALTING\n",
            err, 1);
    #if TEST_MODE_ENABLED
    lapic_timer_driver_test();
    #endif
    #endif

    /* Init time manager */
    #if KERNEL_DEBUG == 1
    kernel_serial_debug("Initializing time manager\n");
    #endif
    #if ENABLE_IO_APIC != 0 && ENABLE_LAPIC_TIMER != 0
    err = time_init(&lapic_timer_driver, &rtc_driver, &pit_driver);
    #else
    err = time_init(&pit_driver, &rtc_driver, NULL);
    #endif
    INIT_MSG("Time Manager Initialized\n",
             "Error while initializing Time Manager: %d. HALTING\n",
            err, 1);
    #if TEST_MODE_ENABLED
    time_ok_test();
    #endif

    #if TEST_MODE_ENABLED
    bios_call_test();
    kernel_queue_test();
    #endif

    /* Init SMP */
    #if KERNEL_DEBUG == 1
    kernel_serial_debug("Initializing SMP\n");
    #endif
    err = smp_init();
    INIT_MSG("", "Error while initializing SMP: %d. HALTING\n",err, 1);

    /* Init keyboard driver */
    #if KERNEL_DEBUG == 1
    kernel_serial_debug("Initializing keyboard driver\n");
    #endif
    err = keyboard_init();
    INIT_MSG("Keyboard Initialized\n",
             "Error while initializing keyboard: %d. HALTING\n",
            err, 1);
    #if TEST_MODE_ENABLED
    rtc_driver_test();
    #endif

    /* Init ATA PIO driver */
    #if KERNEL_DEBUG == 1
    kernel_serial_debug("Initializing ATA PIO driver\n");
    #endif
    err = ata_pio_init();
    INIT_MSG("ATA PIO Initialized\n",
             "Error while initializing ATA PIO: %d.\n",
             err, 0);
    #if TEST_MODE_ENABLED
    ata_pio_driver_test();
    #endif

    /* Init Scheduler */
    #if KERNEL_DEBUG == 1
    kernel_serial_debug("Initializing scheduler\n");
    #endif
    err = sched_init();
    INIT_MSG("",
             "Error while initializing the scheduler: %d.\n",
             err, 0);

    /* We should never reach this code */
    kernel_panic(OS_ERR_UNAUTHORIZED_ACTION);
}
