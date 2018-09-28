#include <Interrupt/exceptions.h>
#include <Interrupt/panic.h>
#include <IO/kernel_output.h>
#include <Cpu/cpu.h>
#include <Tests/test_bank.h>
#include <Drivers/pic.h>

#if EXCEPTION_OK_TEST == 1

static void dummy(cpu_state_t* cpu,  uint32_t int_id,
                                stack_state_t* stack)
{
    (void)cpu;
    (void)int_id;
    (void)stack;
}

void exception_ok_test(void)
{
    OS_RETURN_E err;


    /* TEST REGISTER < MIN */
    if((err = register_exception_handler(MIN_EXCEPTION_LINE - 1, dummy))
     != OR_ERR_UNAUTHORIZED_INTERRUPT_LINE)
    {
        kernel_error("TEST_SW_EXC 0\n");
        kernel_panic(err);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_SW_EXC 0\n");
    }

    /* TEST REGISTER > MAX */
    if((err = register_exception_handler(MAX_EXCEPTION_LINE + 1, dummy))
     != OR_ERR_UNAUTHORIZED_INTERRUPT_LINE)
    {
        kernel_error("TEST_SW_EXC 1\n");
        kernel_panic(err);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_SW_EXC 1\n");
    }

    /* TEST REMOVE < MIN */
    if((err = remove_exception_handler(MIN_EXCEPTION_LINE - 1))
     != OR_ERR_UNAUTHORIZED_INTERRUPT_LINE)
    {
        kernel_error("TEST_SW_EXC 2\n");
        kernel_panic(err);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_SW_EXC 2\n");
    }

    /* TEST REMOVE > MAX */
    if((err = remove_exception_handler(MAX_EXCEPTION_LINE + 1))
     != OR_ERR_UNAUTHORIZED_INTERRUPT_LINE)
    {
        kernel_error("TEST_SW_EXC 3\n");
        kernel_panic(err);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_SW_EXC 3\n");
    }

    /* TEST NULL HANDLER */
    if((err = register_exception_handler(MIN_EXCEPTION_LINE, NULL))
     != OS_ERR_NULL_POINTER)
    {
        kernel_error("TEST_SW_EXC 4\n");
        kernel_panic(err);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_SW_EXC 4\n");
    }

    /* TEST REMOVE WHEN NOT REGISTERED */
    if((err = remove_exception_handler(MIN_EXCEPTION_LINE))
     != OS_NO_ERR)
    {
        kernel_error("TEST_SW_EXC 5\n");
        kernel_panic(err);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_SW_EXC 5\n");
    }
     /* TEST REMOVE WHEN NOT REGISTERED */
    if((err = remove_exception_handler(MIN_EXCEPTION_LINE))
     != OS_ERR_INTERRUPT_NOT_REGISTERED)
    {
        kernel_error("TEST_SW_EXC 7\n");
        kernel_panic(err);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_SW_EXC 7\n");
    }

    /* TEST REGISTER WHEN ALREADY REGISTERED */
    if((err = register_exception_handler(MIN_EXCEPTION_LINE, dummy))
     != OS_NO_ERR)
    {
        kernel_error("TEST_SW_EXC 8\n");
        kernel_panic(err);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_SW_EXC 8\n");
    }

    if((err = register_exception_handler(MIN_EXCEPTION_LINE, dummy))
     != OS_ERR_INTERRUPT_ALREADY_REGISTERED)
    {
        kernel_error("TEST_SW_EXC 9\n");
        kernel_panic(err);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_SW_EXC 9\n");
    }

    /* INIT THINGS */
    if((err = remove_exception_handler(MIN_EXCEPTION_LINE)) != OS_NO_ERR)
    {
        kernel_error("TEST_SW_EXC 10\n");
        kernel_panic(err);
    }
    else 
    {
        kernel_printf("[TESTMODE] TEST_SW_EXC 10\n");
    }

    kernel_printf("[TESTMODE] Software exception tests passed\n");

    while(1)
    {
        __asm__ ("hlt");
    }
}
#else 
void exception_ok_test(void)
{
}
#endif