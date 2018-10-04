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
#include <IO/kernel_output.h>     /* kernel_output() */
#include <BSP/pic.h>              /* pic_init(), pic_driver */
#include <BSP/pit.h>              /* pit_init() */
#include <BSP/rtc.h>              /* rtc_init() */
#include <BSP/acpi.h>             /* acpi_init() */
#include <Interrupt/interrupts.h> /* kernel_interrupt_init() */
#include <Interrupt/exceptions.h> /* kernel_exception_init() */
#include <Interrupt/panic.h>      /* kernel_panic() */
#include <Memory/meminfo.h>       /* memory_map_init() */
#include <Drivers/vesa.h>         /* init_vesa(), vesa_text_vga_to_vesa() */
#include <Lib/string.h>           /* strlen() */

/* RTLK configuration file */
#include <config.h>

#if TEST_MODE_ENABLED
#include <Tests/test_bank.h>
#endif

#define INIT_MSG(msg_success, msg_error, error) {     \
    if(error != OS_NO_ERR)                            \
    {                                                 \
        kernel_error(msg_error, error);               \
        kernel_panic(error);                          \
    }                                                 \
    else if(strlen(msg_success) != 0)                 \
    {                                                 \
        kernel_success(msg_success);                  \
    }                                                 \
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


    #if ENABLE_VESA == 1
    /* Init VESA */
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
    
    kernel_printf("\n==== Kickstarting RTLK ====\n");

    /* Detect CPU */
    #if KERNEL_DEBUG == 1
    kernel_serial_debug("Detecting CPU\n");
    #endif
    err = cpu_detect(1);
    INIT_MSG("", "Error while detecting CPU: %d. HALTING\n", err);

    /* Detect memory */
    #if KERNEL_DEBUG == 1
    kernel_serial_debug("Detecting memory\n");
    #endif
    err = memory_map_init();
    INIT_MSG("", "Error while detecting memory: %d. HALTING\n", err);

    /* Initialize ACPI */
    #if KERNEL_DEBUG == 1
    kernel_serial_debug("Initializing ACPI\n");
    #endif
    err = acpi_init();
    INIT_MSG("ACPI Initialized\n", 
            "Error while initializing ACPI: %d. HALTING\n", 
             err);

    /* Init PIC */
    #if KERNEL_DEBUG == 1
    kernel_serial_debug("Initializing the PIC driver\n");
    #endif
    err = pic_init();
    INIT_MSG("PIC Initialized\n", 
             "Error while initializing PIC: %d. HALTING\n", err);
    #if TEST_MODE_ENABLED
    pic_driver_test();
    #endif

    /* Init kernel's interrupt manager */
    #if KERNEL_DEBUG == 1
    kernel_serial_debug("Initializing the kernel interrupt manager\n");
    #endif
    err = kernel_interrupt_init(&pic_driver);
    INIT_MSG("Kernel Interrupt Manager Initialized\n", 
             "Error while initializing Kernel Interrupt Manager: %d. HALTING\n", 
             err);
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
             err);
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
             err);
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
             err);
    #if TEST_MODE_ENABLED
    rtc_driver_test();
    #endif

    /* Init time manager */
    #if KERNEL_DEBUG == 1
    kernel_serial_debug("Initializing time manager\n");
    #endif
    err = time_init(&pit_driver, &rtc_driver, NULL);
    INIT_MSG("Time Manager Initialized\n", 
             "Error while initializing Time Manager: %d. HALTING\n", 
             err);
    #if TEST_MODE_ENABLED
    time_ok_test();
    #endif

    #if TEST_MODE_ENABLED
    bios_call_test();
    #endif

    kernel_interrupt_restore(1);

    main();

    return;
}
