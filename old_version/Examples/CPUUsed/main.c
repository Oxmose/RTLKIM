#include <Lib/stddef.h>
#include <Lib/stdio.h>
#include <Core/scheduler.h>

/* Used as example, it will be changed in the future. */

int main(void)
{
    uint64_t idle_shed, last_idle_sched = 0;
    uint64_t tmp;
    uint64_t sched_call, last_sched_call = 0;

    uint32_t i = 0;
    for(;;)
    {
        ++i;
        if(i % 1000000 == 0)
        {
            sched_sleep(10);
        }
        if(i % 50000000 == 0)
        {
            idle_shed = sched_get_idle_schedule_count();
            tmp = idle_shed;
            idle_shed -= last_idle_sched;
            last_idle_sched = tmp;
            sched_call = sched_get_schedule_count();
            tmp = sched_call;
            sched_call -= last_sched_call;
            last_sched_call = tmp;

            uint64_t cpu_use = 100 - (idle_shed * 100) / sched_call;
            printf("\r CPU Use: %03u%%    %d %d            ", 
            (uint32_t) cpu_use, (uint32_t)idle_shed, (uint32_t)sched_call);
        }
    }
    return 0;
}
