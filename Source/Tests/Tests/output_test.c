
#include <Lib/stdio.h>
#include <Lib/stdint.h>
#include <IO/kernel_output.h>
#include <Tests/test_bank.h>


#if OUTPUT_TEST  == 1
void output_test(void)
{
    uint32_t i = 0;

    kernel_printf("[TESTMODE] This tag should be empty: %d.\n", i++);

    kernel_error("[TESTMODE] This tag should be ERROR: %d.\n", i++);

    kernel_success("[TESTMODE] This tag should be OK: %d.\n", i++);

    kernel_info("[TESTMODE] This tag should be INFO: %d.\n", i++);

    kernel_debug("[TESTMODE] This tag should be DEBUG: %d.\n", i++);

    kernel_serial_debug("[TESTMODE] This should only out in serial: %d.\n", 
                        i++);
    
    while(1)
    {
        __asm__ ("hlt");
    }
}
#else 
void output_test(void)
{
}
#endif