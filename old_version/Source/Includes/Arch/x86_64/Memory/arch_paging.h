/***************************************************************************//**
 * @file arch_paging.h
 *
 * @see paging.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 25/01/2018
 *
 * @version 1.0
 *
 * @brief Kernel memory paging informations.
 *
 * @details Kernel memory paging informations and structures.
 * 
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __ARCH_PAGING_H_
#define __ARCH_PAGING_H_

#include <Lib/stdint.h> /* Generic int types */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Kernel's page size in Bytes. */
#define KERNEL_PAGE_SIZE 4096

/** @brief Paging structure, not present */
#define PG_STRUCT_ATTR_NOT_PRESENT    0x0000000000000000
/** @brief Paging structure, present */
#define PG_STRUCT_ATTR_PRESENT        0x0000000000000001
/** @brief Paging structure, readable only */
#define PG_STRUCT_ATTR_READ_ONLY      0x0000000000000000
/** @brief Paging structure, readable and writeable */
#define PG_STRUCT_ATTR_READ_WRITE     0x0000000000000002
/** @brief Paging structure, kernel accessible */
#define PG_STRUCT_ATTR_KERNEL_ACCESS  0x0000000000000000
/** @brief Paging structure, user accessible */
#define PG_STRUCT_ATTR_USER_ACCESS    0x0000000000000004
/** @brief Paging structure, write back caches */
#define PG_STRUCT_ATTR_WB_CACHE       0x0000000000000000
/** @brief Paging structure, write through caches */
#define PG_STRUCT_ATTR_WT_CACHE       0x0000000000000008
/** @brief Paging structure, enabled caches */
#define PG_STRUCT_ATTR_ENABLED_CACHE  0x0000000000000000
/** @brief Paging structure, disabled caches */
#define PG_STRUCT_ATTR_DISABLED_CACHE 0x0000000000000010
/** @brief Paging structure, disabled caches */
#define PG_STRUCT_ATTR_NXE            0x1000000000000000
/** @brief Paging structure, 4KB pages */
#define PG_STRUCT_ATTR_4KB_PAGES      0x0000000000000000

/** @brief Paging structure, 1GB pages */
#define PG_PDP_ATTR_1GB_PAGES         0x0000000000000080

/** @brief Paging structure, 2MB pages */
#define PG_PD_ATTR_2MB_PAGES          0x0000000000000080
/** @brief Paging structure, global page */
#define PG_PD_ATTR_GLOBAL             0x0000000000000100

/** @brief PML4 address offset */
#define PML4_OFFSET 39
/** @brief PDPT address offset */
#define PDPT_OFFSET 30
/** @brief PDT address offset */
#define PDT_OFFSET 21
/** @brief PT address offset */
#define PT_OFFSET 12

/* Compatibility layer with i386 */
#define PG_DIR_FLAG_PAGE_SIZE_4KB        PG_STRUCT_ATTR_4KB_PAGES
#define PG_DIR_FLAG_PAGE_SUPER_ACCESS    PG_STRUCT_ATTR_KERNEL_ACCESS
#define PG_DIR_FLAG_PAGE_READ_WRITE      PG_STRUCT_ATTR_READ_WRITE
#define PG_DIR_FLAG_PAGE_READ_ONLY       PG_STRUCT_ATTR_READ_ONLY

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* None */

#endif /* __ARCH_PAGING_H_ */