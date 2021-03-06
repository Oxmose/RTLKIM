#include <IO/kernel_output.h>
#include <Core/scheduler.h>
#include <Interrupt/interrupts.h>
#include <Cpu/panic.h>
#include <Tests/test_bank.h>

#if SCHEDULER_LOAD_TEST == 1

static void* print_th(void*args)
{
    int i = 0;
    int val = (int)args;
    for(i = 0; i < 2; ++i)
    {
        kernel_interrupt_disable();
        kernel_printf("%d ", val % 64);
        kernel_interrupt_restore(1);
        sched_sleep(1000);
    }
    return NULL;
}

void scheduler_load_test(void)
{
    thread_t thread[2048];
    OS_RETURN_E err;

    kernel_interrupt_disable();

    kernel_printf("[TESTMODE] Scheduler tests sarts\n");

    for(int i = 0; i < 1024; ++i)
    {
        err = sched_create_kernel_thread(&thread[i], (63 - (i % 64)), "test",
                                  1024, 0, print_th, (void*)i);
        if(err != OS_NO_ERR)
        {
            kernel_error("Cannot create threads %d\n", err);
            kernel_panic(err);
        }
    }
    kernel_printf("[TESTMODE] ");

    kernel_interrupt_restore(1);

    for(int i = 0; i < 1024; ++i)
    {
        sched_wait_thread(thread[i], NULL, NULL);
    }

    kernel_printf("\n[TESTMODE] Scheduler thread load tests passed\n");

    kernel_interrupt_disable();
}
#else
void scheduler_load_test(void)
{

}
#endif