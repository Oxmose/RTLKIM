/******************************************************************************
 * @file arch_paging.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 25/01/2018
 *
 * @version 1.0
 *
 * @brief x86_64 kernel memory paging informations.
 *
 * @details x86_64 kernel memory paging informations and structures.
 * 
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __X86_64_ARCH_PAGING_H_
#define __X86_64_ARCH_PAGING_H_

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Kernel's page size in Bytes. */
#define KERNEL_PAGE_SIZE 4096

/** @brief Kernel's page alignement mask. */
#define PAGE_ALIGN_MASK (~(KERNEL_PAGE_SIZE - 1))

/** @brief Kernel's page directory entry count. */
#define KERNEL_P4_SIZE 512

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
/** @brief Paging structure, disabled execution */
#define PG_STRUCT_ATTR_NXE            0x8000000000000000
/** @brief Paging structure, 4KB pages */
#define PG_STRUCT_ATTR_4KB_PAGES      0x0000000000000000
/** @brief Custom define flag: hardware mapped. */
#define PG_STRUCT_ATTR_HARDWARE       0x00000200

/** @brief Paging structure, 1GB pages */
#define PG_PDP_ATTR_1GB_PAGES         0x0000000000000080

/** @brief Paging structure, 2MB pages */
#define PG_PD_ATTR_2MB_PAGES          0x0000000000000080
/** @brief Paging structure, global page */
#define PG_PD_ATTR_GLOBAL             0x0000000000000100

/** @brief PML4 address offset */
#define P4_OFFSET 39
/** @brief PDPT address offset */
#define P3_OFFSET 30
/** @brief PDT address offset */
#define P2_OFFSET 21
/** @brief PT address offset */
#define P1_OFFSET 12

/** @brief Recursive paging address base for P3. */
#define P3_RECUR_BASE_ADDR 0xFFFFFFFFFFE00000
/** @brief Recursive paging address base for P2. */
#define P2_RECUR_BASE_ADDR 0xFFFFFFFFC0000000
/** @brief Recursive paging address base for P1. */
#define P1_RECUR_BASE_ADDR 0xFFFFFF8000000000

/** @brief Architecture maximal address. */
#define ARCH_MAX_ADDRESS 0xFFFFFFFFFFFFFFFF

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/* None */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* None */

#endif /* #ifndef __X86_64_ARCH_PAGING_H_ */