#include <Lib/stdio.h>
#include <BSP/rtc.h>
#include <Core/scheduler.h>

/* Used as example, it will be changed in the future. */
int main(void)
{    
    printf("\n");
    while(1)
    {
        date_t date;
        uint32_t time, hours, minutes, seconds;
        time = rtc_get_current_daytime();
        hours = time / 3600;
        minutes = (time / 60) % 60;
        seconds = time % 60;
        date = rtc_get_current_date();
        printf("\rDate: %02u/%02u/%02u | Time: %02u:%02u:%02u", date.day, date.month, date.year, hours, minutes, seconds);
        sched_sleep(200);
    }
    return 0;
}