/***************************************************************************//**
 * @file meminfo.h
 * 
 * @see meminfo.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 25/01/2018
 *
 * @version 1.0
 *
 * @brief Kernel memory detector.
 * 
 * @details This module is used to detect the memory mapping of the system.
 * 
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __MEMINFO_H_
#define __MEMINFO_H_

#include <Lib/stddef.h> /* OS_RETURN_E */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/*******************************************************************************
* STRUCTURES
******************************************************************************/

/** @brief Defines a memory range with its type as defined by the multiboot 
 * standard.
 */
struct mem_range
{
    /** @brief Range's base address. */
    uint32_t base;

    /** @brief Range's limit. */
    uint32_t limit;

    /** @brief Range's memory type. */
    uint32_t type;
};

/** 
 * @brief Defines mem_range_t type as a shorcut for struct mem_range.
 */
typedef struct mem_range mem_range_t;

/*******************************************************************************
* FUNCTIONS
******************************************************************************/

/** 
 * @brief Inititalizes the kernel's memory map.
 * 
 * @brief Initializes the kernel's memory map while detecting the system's
 * memory organization.
 * 
 * @return The succes state or the error code. 
 * - OS_NO_ERR is returned if no error is encountered. 
 * - No other return value is possible.
 */
OS_RETURN_E memory_map_init(void);

#endif /* __MEMINFO_H_ */