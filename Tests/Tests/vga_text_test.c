#include <Interrupt/interrupts.h>
#include <Cpu/panic.h>
#include <IO/kernel_output.h>
#include <Cpu/cpu.h>
#include <Drivers/vga_text.h>
#include <Tests/test_bank.h>

#if VGA_TEXT_TEST == 1
void vga_text_test(void)
{
    vga_put_string("[TESTMODE]");
    for(uint16_t i = 32; i < 127; ++i)
    {
        vga_put_char((char)i);
    }
    for(uint16_t i = 0; i < 256; ++i)
    {
        if(i%16 == 0)
        {
            vga_put_char('\n');
            vga_put_string("[TESTMODE]");
        }
        colorscheme_t color = {
            .vga_color = 1,
            .foreground = i & 0x0F,
            .background = i & 0xF0
        };
        vga_set_color_scheme(color);
        vga_put_char('A');
    }
    vga_put_char('\n');


    while(1)
    {
        __asm__ ("hlt");
    }
}
#else
void vga_text_test(void)
{

} 
#endif