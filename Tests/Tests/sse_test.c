#include <Core/scheduler.h>
#include <Lib/stdio.h>
#include <BSP/lapic.h>
#include <Sync/mutex.h>
#include <Sync/semaphore.h>
#include <Memory/kheap.h>
#include <Tests/test_bank.h>

#if SSE_TEST == 1
static uint8_t arrayTabF[256];
static uint8_t arrayTabT[256];

mutex_t mut;

void testsse(void)
{
    __asm__ __volatile__ (
        "movups (%0), %%xmm0\n\t" 
        "movntdq %%xmm0, (%1)\n\t"
    ::"r"(arrayTabF), "r"(arrayTabT) : "memory");
}

void sse_test(void)
{
    kernel_interrupt_disable();
    testsse();
    printf("[TESTMODE] SSE 1 passed\n");
    testsse();
    printf("[TESTMODE] SSE 2 passed\n");
    testsse();
    printf("[TESTMODE] SSE 3 passed\n");
    kernel_interrupt_restore(1);
    sched_sleep(100);
    kernel_interrupt_disable();
    testsse();
    kernel_interrupt_restore(1);
    printf("[TESTMODE] SSE 4 passed\n");
    sched_sleep(100);  
    kernel_interrupt_disable();  
    testsse();
    kernel_interrupt_restore(1);
    printf("[TESTMODE] SSE 5 passed\n");
}
#else
void sse_test(void)
{

}
#endif