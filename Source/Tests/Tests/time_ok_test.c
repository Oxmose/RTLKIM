#include <Interrupt/interrupts.h>
#include <Interrupt/panic.h>
#include <IO/kernel_output.h>
#include <Cpu/cpu.h>
#include <BSP/rtc.h>
#include <Tests/test_bank.h>

#if TIME_OK_TEST == 1
void time_ok_test(void)
{
    uint32_t tick_count = time_get_tick_count();
    uint32_t daytime = rtc_get_current_daytime();
    uint32_t new_tick_count;
    uint32_t new_daytime;


    kernel_interrupt_restore(1);

    for(volatile uint32_t i = 0; i < 500000000; ++i);

    new_tick_count = time_get_tick_count();
    new_daytime = rtc_get_current_daytime();

    if(tick_count != new_tick_count &&
       daytime != new_daytime)
    {
        kernel_printf("[TESTMODE] TIME tests passed\n");;
    }
    else 
    {
        kernel_error("Time test failed (%d %d) (%d %d)\n",
                      tick_count, daytime, new_tick_count, new_daytime);
    }

    kernel_interrupt_disable();
     while(1)
    {
        __asm__ ("hlt");
    }
}
#else
void time_ok_test(void)
{

} 
#endif