#include <IO/kernel_output.h>
#include <Core/scheduler.h>
#include <Interrupt/interrupts.h>
#include <Interrupt/panic.h>
#include <Tests/test_bank.h>
#include <Lib/string.h>

#if DIV_BY_ZERO_TEST == 1

static void* thread_func(void* args)
{
    volatile int m = 50 / (int)args;
    (void)m;
    return NULL;
}

void div_by_zero_test(void)
{
    thread_t thread;
    uint32_t term, cause;
    OS_RETURN_E err;


    err = sched_create_kernel_thread(&thread, 5, "test", 
                                1024, thread_func, (void*)0);
    if(err != OS_NO_ERR)
    {
        kernel_error("Cannot create threads %d\n", err);
        kernel_panic(err);
    }

    sched_wait_thread(thread, (void**)&term, &cause);

    kernel_printf("[TESTMODE] Thread termination: %d, cause %d\n", term, cause);
    switch(cause)
    {
        case 0:
            kernel_printf("[TESTMODE] Normal exit. \n");
            break;
        case 1:
            kernel_printf("[TESTMODE] Division by zero. \n");
            break;
        case 2:
            kernel_printf("[TESTMODE] Panic. \n");
            break;
    }
}
#else
void div_by_zero_test(void)
{

} 
#endif