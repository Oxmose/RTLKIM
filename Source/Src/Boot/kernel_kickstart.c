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
 * Init the rest of the kernel after GDT, IDT and TSS
 * AT THIS POINT INTERRUPT SHOULD BE DISABLED
 ******************************************************************************/


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
    return;
}
