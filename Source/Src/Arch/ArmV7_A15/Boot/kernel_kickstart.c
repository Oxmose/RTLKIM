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

/* RTLK configuration file */
#include <config.h>

#if TEST_MODE_ENABLED
#include <Tests/test_bank.h>
#endif

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/* None. */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

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
    volatile unsigned int* uart = (volatile unsigned int*)0x1c090000;
    *uart = 'H';
    *uart = 'e';
    *uart = 'l';
    *uart = 'l';
    *uart = 'o';
    *uart = ' ';
    *uart = 'W';
    *uart = 'o';
    *uart = 'r';
    *uart = 'l';
    *uart = 'd';
    *uart = '!';
    while(1);
}
