#include <Time/time_management.h>
#include <Lib/stdio.h>

int main(void)
{
    while(1)
    {
        printf("Waiting 500ms (Uptime: %u)\n",
               (uint32_t)time_get_current_uptime());
        time_wait_no_sched(500);
    }
}