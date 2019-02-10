#include <Core/scheduler.h>
#include <Lib/stdio.h>

/* Used as example, it will be changed in the future. */
int main(void)
{
    while(1)
    {
        printf(".");
        sched_sleep(1000);
    }
    return 0;
}
