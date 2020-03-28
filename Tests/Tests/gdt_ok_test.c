
#include <Lib/stdio.h>
#include <Lib/stdint.h>
#include <Lib/stddef.h>
#include <Cpu/cpu_settings.h>

#include <Tests/test_bank.h>

/* Kernel GDT structure */
extern gdt_ptr_t cpu_gdt_ptr;

#if GDT_OK_TEST  == 1
void gdt_ok_test(void)
{
    printf("[TESTMODE][OK] GDT size 0x%08x\n", cpu_gdt_ptr.size);
    printf("[TESTMODE][OK] GDT base 0x%08x\n", cpu_gdt_ptr.base);
   
    printf("[TESTMODE][OK] GDT size desc 0x%08x\n", (address_t)&cpu_gdt_ptr.size);
    printf("[TESTMODE][OK] GDT base desc 0x%08x\n", (address_t)&cpu_gdt_ptr.base);
    
    while(1)
    {
        __asm__ ("hlt");
    }
}
#else 
void gdt_ok_test(void)
{
}
#endif