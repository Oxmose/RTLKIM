
#include <Lib/stdio.h>
#include <Lib/stdint.h>
#include <Lib/stddef.h>
#include <IO/kernel_output.h>
#include <Tests/test_bank.h>
#include <Memory/kheap.h>

#if KHEAP_TEST  == 1
void kheap_test(void)
{
    uint32_t i;
    void* address[20] = {NULL};
    uint32_t sizes[20];

    for(i = 0; i < 20; ++i)
    {
        sizes[i] = sizeof(int32_t) * (i + 1);
        address[i] = kmalloc(sizes[i]);
        
    }
    for(i = 0; i < 20; ++i)
    {
        if(i == 5 || i == 10)
        {
            kernel_printf("\n");
        }
        kernel_printf("[TESTMODE] Kheap 0x%08x -> %dB\n", address[i], sizes[i]);
    }
    for(i = 5; i < 10; ++i)
    {
        kfree(address[i]);
        
    }
    for(i = 5; i < 10; ++i)
    {
        sizes[i] = sizeof(int32_t) * (i + 2);
        address[i] = kmalloc(sizes[i]);        
    }
    kernel_printf("\n");
    for(i = 0; i < 20; ++i)
    {
        if(i == 5 || i == 10)
        {
            kernel_printf("\n");
        }
        kernel_printf("[TESTMODE] Kheap 0x%08x -> %dB\n", address[i], sizes[i]);
    }
    
    while(1)
    {
        __asm__ ("hlt");
    }
}
#else 
void kheap_test(void)
{
}
#endif