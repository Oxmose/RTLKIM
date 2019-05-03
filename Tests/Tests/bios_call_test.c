#include <Interrupt/interrupts.h>
#include <IO/kernel_output.h>
#include <Cpu/cpu.h>
#include <BSP/bios_call.h>
#include <Tests/test_bank.h>
#include <IO/graphic.h>
#include <Drivers/vga_text.h>
#include <Lib/string.h>

#if BIOS_CALL_TEST == 1
void bios_call_test(void)
{
    uint32_t i;
    bios_int_regs_t regs;
    char* str = "BIOS Real Mode Calls tests passed\n\0";

    cursor_t cursor;
    vga_save_cursor(&cursor);

    /* Define cursor position */
    regs.ax = 0x0200;
    regs.bx = 0x0000;
    regs.dx = ((cursor.x & 0xFF)) | ((cursor.y & 0xFF)) << 8;

    bios_call(0x10, &regs);

    /* Write srtring with bios */
    for(i = 0; i < strlen(str); ++i)
    {
        regs.ax = 0x0E00;
        regs.ax |= str[i] & 0x00FF;
        regs.bx = 0x0000;
        regs.cx = 0x0001;

        bios_call(0x10, &regs);
    }

    kernel_printf("\n");
    kernel_printf("[TESTMODE] Bios call success");

    while(1);
    
}
#else
void bios_call_test(void)
{

} 
#endif