/*******************************************************************************
 *
 * File: kernel_kickstart.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 15/12/2017
 *
 * Version: 1.0
 *
 * Initializes the rest of the kernel after GDT, IDT and TSS initialization.
 * AT THIS POINT INTERRUPT SHOULD BE DISABLED
 ******************************************************************************/

#include <Cpu/cpu.h>          /* detect_cpu() */
#include <IO/kernel_output.h> /* kernel_output() */

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Init each basic drivers for the kernel, then init the scheduler and
 * start the system.
 */
void kernel_kickstart(void)
{
    kernel_printf("------------------------------ Kickstarting RTLK -------------------------------");

    /* Launch CPU detection routine */
    if(detect_cpu(1) != OS_NO_ERR)
    {
        kernel_error("Could not detect CPU, halting.");
        return;
    }

    return;
}
