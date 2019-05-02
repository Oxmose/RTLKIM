

#include <Interrupt/interrupts.h>
#include <Cpu/panic.h>
#include <IO/kernel_output.h>
#include <Cpu/cpu.h>
#include <BSP/lapic.h>
#include <Tests/test_bank.h>

#if LAPIC_DRIVER_TEST == 1
void lapic_driver_test(void)
{
    OS_RETURN_E err;

    /* TEST OEI > MAX */
    if((err = lapic_set_int_eoi(MAX_INTERRUPT_LINE + 1)) !=
       OS_ERR_NO_SUCH_IRQ_LINE)
    {
        kernel_error("TEST_LAPIC 0\n");
        kernel_panic(err);
    }

    kernel_debug("[TESTMODE] Local APIC tests passed\n");
}
#else
void lapic_driver_test(void)
{

} 
#endif