
#include <Lib/stdio.h>

#include <Tests/test_bank.h>

#if LOADER_OK_TEST  == 1
void loader_ok_test(void)
{
    printf("[TESTMODE][OK] RTLK Loaded correctly.\n");
    while(1)
    {
        __asm__ ("hlt");
    }
}
#else 
void loader_ok_test(void)
{
}
#endif