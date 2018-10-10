#include <Lib/stdio.h>
#include <BSP/rtc.h>
#include <Core/scheduler.h>

/* Used as example, it will be changed in the future. */
int main(void)
{    
    uint32_t time, hours, minutes, seconds;
    date_t date;
    printf("\n");
    while(1)
    {
        time = rtc_get_current_daytime();
        hours = time / 3600;
        minutes = (time / 60) % 60;
        seconds = time % 60;
        date = rtc_get_current_date();
        printf("\rDate: %02d/%02d/%02d | Time: %02d:%02d:%02d", date.day, date.month, date.year, hours, minutes, seconds);
        sched_sleep(200);
    }
    return 0;
}