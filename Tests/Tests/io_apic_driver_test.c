/*******************************************************************************
 *
 * File: test_pic.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 09/01/2018
 *
 * Version: 1.0
 *
 * Kernel tests bank: Programmable interrupt controler tests
 ******************************************************************************/

/*
 * !!! THESE TESTS MUST BE DONE BEFORE INITIALIZING ANY INTERRUPT HANDLER
 *     BETWEEN MIN_INTERRUPT_LINE AND MAX_INTERRUPT_LINE !!!
 * !!! THESE TESTS MUST BE DONE AFTER INITIALIZING THE PIC AND BEFORE THE
 *     IOAPIC!!!
 */

#include <Interrupt/interrupts.h>
#include <IO/kernel_output.h>
#include <Cpu/cpu.h>
#include <BSP/io_apic.h>
#include <Tests/test_bank.h>
#include <Cpu/panic.h>

#if IO_APIC_DRIVER_TEST == 1
void io_apic_driver_test(void)
{
    OS_RETURN_E err;

    /* TEST MASK > MAX */
    if((err = io_apic_set_irq_mask(255, 0)) !=
       OS_ERR_NO_SUCH_IRQ_LINE)
    {
        kernel_error("TEST_IOAPIC 0\n");
        kernel_panic(err);
    }

    /* TEST MASK <= MAX */
    if((err = io_apic_set_irq_mask(IO_APIC_MAX_IRQ_LINE , 1)) != OS_NO_ERR)
    {
        kernel_error("TEST_IOAPIC 1\n");
        kernel_panic(err);
    }

    if((err = io_apic_set_irq_mask(IO_APIC_MAX_IRQ_LINE , 0)) != OS_NO_ERR)
    {
        kernel_error("TEST_IOAPIC 2\n");
        kernel_panic(err);
    }


    kernel_debug("[TESTMODE] IO-APIC tests passed\n");
}
#else 
void io_apic_driver_test(void)
{
}
#endif