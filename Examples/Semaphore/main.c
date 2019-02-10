#include <Lib/stdio.h>
#include <Sync/semaphore.h>
#include <Core/scheduler.h>

#define THREAD_COUNT 100

semaphore_t sem;

void* thread_routine(void* args)
{
    while(1)
    {
        sem_pend(&sem);
        printf("%d ", (int)args);
    }

    return NULL;
}

int main(void)
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
        err = sched_create_kernel_thread(&threads[i], i % 10, "sem_ex", 512, 0,
                                  thread_routine, (void*)i);
        if(err != OS_NO_ERR)
        {
            printf("Error while creating thread %d: %d\n", i, err);
            return -1;
        }
    }

    while(1)
    {
        sched_sleep(100);
        sem_post(&sem);
    }
    return 0;
}