/***************************************************************************//**
 * @file cpu_api.h
 *
 * @see cpu_api.c
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

#ifndef __CPU_API_H_
#define __CPU_API_H_

#include <Lib/stdint.h>           /* Generic int types */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

 /* None */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Returns the current CPU id.
 * 
 * @details The function returns the CPU id on which the call is made.
 *
 * @return The current CPU id is returned on succes and -1 is return in case 
 * of error.
 */
int32_t cpu_get_id(void);

#endif /* __CPU_API_H_ */