#include <IO/kernel_output.h>
#include <Core/scheduler.h>
#include <Interrupt/interrupts.h>
#include <Interrupt/panic.h>
#include <Time/time_management.h>
#include <Tests/test_bank.h>

#if SCHEDULER_SLEEP_TEST == 1

static void* print_th(void*args)
{
    (void) args;
    uint64_t i = time_get_current_uptime();
    sched_sleep(200);
    if(time_get_current_uptime() < i + 200)
    {
        kernel_error("Scheduler thread sleep tests failed\n");
    }
    else 
    {
        kernel_printf("[TESTMODE] Scheduler thread sleep tests passed\n");
    }
    return NULL;
}

void scheduler_sleep_test(void)
{
    thread_t thread;
    OS_RETURN_E err;

    kernel_interrupt_restore(1);

    kernel_printf("[TESTMODE] Scheduler tests sarts\n");

    err = sched_create_thread(&thread, 0, "test", 
                                  1024, print_th, NULL);
    if(err != OS_NO_ERR)
    {
        kernel_error("Cannot create threads %d\n", err);
        kernel_panic(err);
    }

    sched_wait_thread(thread, NULL, NULL);

    kernel_interrupt_disable();
}
#else
void scheduler_sleep_test(void)
{

} 
#endif