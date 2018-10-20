/***************************************************************************//**
 * @file paging_alloc.h
 *
 * @see paging_alloc.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 20/10/2018
 *
 * @version 1.0
 *
 * @brief Kernel memory physical frame and virtual page allocator.
 *
 * @details Kernel memory physical frame and virtual page allocator. This module
 * allows to* allocate and deallocate physical memory frame and virtual pages.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __PAGING_ALLOC_H_
#define __PAGING_ALLOC_H_

#include <Lib/stddef.h> /* OS_RETURN_E */
#include <Lib/stdint.h> /* Generic int types */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

struct mem_area
{
    uint32_t start;

    uint32_t size;

    struct mem_area* next;
    struct mem_area* prev;
};

typedef struct mem_area mem_area_t;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

OS_RETURN_E paging_alloc_init(void);

void* kernel_paging_alloc_frame(OS_RETURN_E* err);

OS_RETURN_E kernel_paging_free_frame(void* frame_addr);

void* kernel_paging_alloc_page(OS_RETURN_E* err);

OS_RETURN_E kernel_paging_free_page(void* page_addr);

#endif /* __PAGING_ALLOC_H_ */