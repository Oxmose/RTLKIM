
#include <Lib/stdio.h>
#include <Lib/stdint.h>
#include <Cpu/cpu_settings.h>

#include <Tests/test_bank.h>

/* Kernel IDT structure */
extern uint16_t cpu_idt_size;
extern uint32_t cpu_idt_base;

#if IDT_OK_TEST  == 1
void idt_ok_test(void)
{
    printf("[TESTMODE][OK] IDT base 0x%08x\n", cpu_idt_base);
    printf("[TESTMODE][OK] IDT size 0x%08x\n", (uint32_t)cpu_idt_size);
   
    printf("[TESTMODE][OK] IDT size desc 0x%08x\n", (address_t)&cpu_idt_size);
    printf("[TESTMODE][OK] IDT base desc 0x%08x\n", (address_t)&cpu_idt_base);
    
    while(1)
    {
        __asm__ ("hlt");
    }
}
#else 
void idt_ok_test(void)
{
}
#endif