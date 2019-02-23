#include <Core/scheduler.h>
#include <Lib/stdio.h>
#include <BSP/lapic.h>
#include <Sync/mutex.h>
#include <Memory/kheap.h>

#define TAB_SIZE 1000000

static uint8_t* arrayTab;
static uint32_t arrayVal[MAX_CPU_COUNT];

mutex_t mut;
void* thread_routine(void* args)
{
    uint32_t index = (int)args;
    for(uint32_t k = 0; k < 1600 / MAX_CPU_COUNT; ++k)
    {
        for(uint32_t i = 0; i < TAB_SIZE; ++i)
        {
            arrayVal[index] += arrayTab[i];
        }
    }

    return NULL;
}

/* Used as example, it will be changed in the future. */
int main(void)
{
    thread_t threads[MAX_CPU_COUNT];
    uint32_t start_time;

    kernel_printf("Starting Main\n");

    arrayTab = kmalloc(sizeof(uint8_t) * TAB_SIZE);

    start_time = time_get_current_uptime();
    for(uint32_t k = 0; k < 1600; ++k)
    {
        for(uint32_t i = 0; i < TAB_SIZE; ++i)
        {
            arrayVal[0] += arrayTab[i];
        }
    }
    printf("Single core took: %u\n", (uint32_t)time_get_current_uptime() -  start_time);



    mutex_init(&mut, MUTEX_FLAG_NONE, MUTEX_PRIORITY_ELEVATION_NONE);

    start_time = time_get_current_uptime();

    for(uint32_t i = 0; i < MAX_CPU_COUNT; ++i)
    {
        sched_create_kernel_thread(&threads[i], 0, "thread T", 4096,
                                   i, thread_routine, (void*)i);
        sched_sleep(100);
    }
    for(uint32_t i = 0; i < MAX_CPU_COUNT; ++i)
    {
        sched_wait_thread(threads[i], NULL, NULL);
    }

    printf("Multi core took: %u\n", (uint32_t)time_get_current_uptime() -  start_time);

    return 0;
}
