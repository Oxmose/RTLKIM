/*******************************************************************************
 *
 * File: smp.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 27/03/2018
 *
 * Version: 1.0
 *
 * SMP implementation of the kernel. The different functions in this File
 * allow the systen to detect, initialize and manage CPU cores.
 ******************************************************************************/

#include <Lib/stdint.h>            /* Generic int types */
#include <Lib/stddef.h>            /* OS_RETURN_E */

/* RTLK configuration file */
#include <config.h>

/* Header file */
#include <Cpu/smp.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

OS_RETURN_E smp_init(void)
{
    return OS_ERR_NOT_SUPPORTED;
}

void smp_ap_core_init(void)
{
    return;

}

uint32_t smp_get_booted_cpu_count(void)
{
    return 1;
}