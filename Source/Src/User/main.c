#include <Lib/stdio.h>
#include <BSP/rtc.h>
#include <Cpu/cpu.h>

/* Used as example, it will be changed in the future. */
int main(void)
{
    printf("\n");
    while(1)
    {
        uint32_t time = rtc_get_current_daytime();
        uint32_t hours = time / 3600;
        uint32_t minutes = (time % 3600) / 60;
        uint32_t seconds = time % 60;

        date_t date = rtc_get_current_date();


        printf("\r\tTime is: %02d:%02d:%02d %02d/%02d/%04d   ", 
                hours, minutes, seconds, 
                date.day, date.month, date.year);
        cpu_hlt();
    }
    return 0;
}
