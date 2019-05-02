/***************************************************************************//**
 * @file cpu_api.c
 *
 * @see cpu_api.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 02/05/2019
 *
 * @version 1.0
 *
 * @brief X86 CPU API functions.
 *
 * @details X86 CPU API. Gathers generic API calls to manipulate the CPU
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <Lib/stdint.h>       /* Generic int types */
#include <BSP/lapic.h>        /* lapic_get_id() */

/* RTLK configuration file */
#include <config.h>

/* Header file */
#include <API/cpu_api.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/* None */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

int32_t cpu_get_id(void)
{
    /* If lapic is not activated but we only use one CPU */
    if(MAX_CPU_COUNT == 1)
    {
        return 0;
    }    
    return lapic_get_id();
}