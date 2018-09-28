
#include <Lib/stdio.h>
#include <Lib/stdint.h>
#include <IO/kernel_output.h>
#include <Tests/test_bank.h>
#include <Interrupt/panic.h>


#if PANIC_TEST  == 1
void panic_test(void)
{
    kernel_panic(666);
    
    while(1)
    {
        __asm__ ("hlt");
    }
}
#else 
void panic_test(void)
{
}
#endif