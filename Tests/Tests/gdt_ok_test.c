
#include <Lib/stdio.h>
#include <Lib/stdint.h>
#include <Cpu/cpu_settings.h>

#include <Tests/test_bank.h>

/* Kernel GDT structure */
extern uint16_t cpu_gdt_size;
extern uint32_t cpu_gdt_base;

#if GDT_OK_TEST  == 1
void gdt_ok_test(void)
{
    printf("[TESTMODE][OK] GDT base 0x%08x\n", cpu_gdt_base);
    printf("[TESTMODE][OK] GDT size 0x%08x\n", (uint32_t)cpu_gdt_size);
   
    printf("[TESTMODE][OK] GDT size desc 0x%08x\n", (address_t)&cpu_gdt_size);
    printf("[TESTMODE][OK] GDT base desc 0x%08x\n", (address_t)&cpu_gdt_base);
    
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