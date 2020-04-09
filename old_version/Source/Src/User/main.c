#include <Core/scheduler.h>
#include <Lib/stdio.h>
#include <Drivers/lapic.h>
#include <Sync/mutex.h>
#include <Sync/semaphore.h>
#include <Memory/kheap.h>
#include <Drivers/vesa.h>

#define TAB_SIZE 1000000
#define THREAD_COUNT 10

semaphore_t sem;
static uint8_t* arrayTab;
static uint32_t arrayVal[MAX_CPU_COUNT];

mutex_t mut;
void* thread_routine(void* args)
{
    uint32_t index = (address_t)args;
    for(uint32_t k = 0; k < 1600 / MAX_CPU_COUNT; ++k)
    {
        for(uint32_t i = 0; i < TAB_SIZE; ++i)
        {
            arrayVal[index] += arrayTab[i];
        }
    }

    return NULL;
}


void* sem_thread_routine(void* args)
{
    while(1)
    {
        sem_pend(&sem);
        printf("%u ", (uint32_t)(address_t)args);
    }

    return NULL;
}

int sem_ex(void)
{
    printf("\n");

    OS_RETURN_E err;

    thread_t threads[THREAD_COUNT];
    uint32_t i;

    err = sem_init(&sem, 0);
    if(err != OS_NO_ERR)
    {
        printf("Error while creating semaphore: %d\n", err);
        return -1;
    }

    for(i = 0; i < THREAD_COUNT; ++i)
    {
        err = sched_create_kernel_thread(&threads[i], i % 10, "sem_ex", 1024, 0,
                                  sem_thread_routine, (void*)(address_t)i);
        if(err != OS_NO_ERR)
        {
            printf("Error while creating thread %u: %d\n", i, err);
            return -1;
        }
    }

    while(1)
    {
        sched_sleep(50);
        sem_post(&sem);
    }
    return 0;
}


/* Used as example, it will be changed in the future. */
int main(void)
{
    thread_t threads[MAX_CPU_COUNT];
    uint32_t start_time;

    kernel_printf("Starting Main\n");


    arrayTab = kmalloc(sizeof(uint8_t) * TAB_SIZE);
    if(arrayTab == NULL)
    {
        printf("ERROR Could not allocate memory");
        return -1;
    }

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
                                   i, thread_routine, (void*)(address_t)i);
    }
    for(uint32_t i = 0; i < MAX_CPU_COUNT; ++i)
    {
        sched_wait_thread(threads[i], NULL, NULL);
    }

    printf("Multi core took: %u\n", (uint32_t)time_get_current_uptime() -  start_time);


    sem_ex();

    return 0;
}