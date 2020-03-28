#include <Core/scheduler.h>
#include <Sync/critical.h>
#include <IO/kernel_output.h>
#include <Lib/stdio.h>
#include <Tests/test_bank.h>

#if SPINLOCK_TEST == 1
static thread_t thread_mutex1;
static thread_t thread_mutex2;

static spinlock_t lock = SPINLOCK_INIT_VALUE;

static volatile uint32_t lock_res;


void* spin_thread_1(void *args)
{
    for(int i = 0; i < 200000; ++i)
    {
        pause_spinlock(&lock.value);

        uint32_t tmp = lock_res;
        for(volatile uint32_t k = 0; k < 200; ++k);

        lock_res = tmp + 1;

        lock.value = 0;
    }

    (void )args;
    return NULL;
}

void spinlock_test(void)
{
    OS_RETURN_E err;

    lock_res = 0;

    if(sched_create_kernel_thread(&thread_mutex1, 1, "thread1", 1024, 0, spin_thread_1, NULL) != OS_NO_ERR)
    {
        kernel_error(" Error while creating the main 1 thread!\n");
        return;
    }
    if(sched_create_kernel_thread(&thread_mutex2, 1, "thread1", 1024, 1, spin_thread_1, NULL) != OS_NO_ERR)
    {
        kernel_error(" Error while creating the main 2 thread!\n");
        return;
    }

    if((err = sched_wait_thread(thread_mutex1, NULL, NULL)) != OS_NO_ERR)
    {
        kernel_error("Error while waiting thread! [%d]\n", err);
        return;
    }
    if(sched_wait_thread(thread_mutex2, NULL, NULL) != OS_NO_ERR)
    {
        kernel_error("Error while waiting thread! [%d]\n", err);
        return;
    }

    printf("[TESTMODE]Lock res = %u\n", lock_res);
    if(lock_res == 400000)
    {
        printf("[TESTMODE] Spinlock test passed.\n");
    }
}
#else
void spinlock_test(void)
{
}
#endif