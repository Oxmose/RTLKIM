/***************************************************************************//**
 * @file kernel_kickstart.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 03/05/2019
 *
 * @version 1.0
 *
 * @brief Kernel's main boot sequence.
 *
 * @warning At this point interrupts should be disabled.
 *
 * @details Kernel's booting sequence. Initializes the rest of the kernel after
 * basic initialization. Initializes the hardware and software
 * core of the kernel.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <IO/kernel_output.h>
#include <Cpu/panic.h>

/* UTK configuration file */
#include <config.h>

#if TEST_MODE_ENABLED
#include <Tests/test_bank.h>
#endif

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

void test(cpu_state_t *cpu_state, uint32_t int_id, stack_state_t* stack_state)
{
    for(uint32_t i = 0; i < 13; ++i)
    {
        kernel_printf("\n==== SWI 0x%x! ====\n", cpu_state->registers[i]);
    }
    kernel_printf("\n==== SWI 0x%x! ====\n", cpu_state->sp);
    kernel_printf("\n==== SWI 0x%x! ====\n", cpu_state->lr);
    kernel_printf("\n==== SWI 0x%x! ====\n", cpu_state->pc);
    kernel_printf("\n==== SWI INT %d! ====\n", int_id);
    kernel_printf("\n==== SWI CSPR %d! ====\n", stack_state->cspr);
}

/**
 * @brief Main boot sequence, C kernel entry point.
 *
 * @details Main boot sequence, C kernel entry point. Initializes each basic
 * drivers for the kernel, then init the scheduler and start the system.
 *
 * @warning This function should never return. In case of return, the kernel
 * should be able to catch the return as an error.
 */
void kernel_kickstart(void)
{
    kernel_printf("\n==== Kickstarting UTK ====\n");

    kernel_panic(1);

    while(1);
}
