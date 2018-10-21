/***************************************************************************//**
 * @file paging.h
 *
 * @see paging.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 25/01/2018
 *
 * @version 1.0
 *
 * @brief Kernel memory paging manager.
 *
 * @details Kernel memory paging manager. This module allows to enable or
 * disable paging in the kernel. The memory mapping functions are also located
 * here. The module also defines the page's size.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __PAGING_H_
#define __PAGING_H_

#include <Lib/stddef.h> /* OS_RETURN_E */
#include <Lib/stdint.h> /* Generic int types */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Kernel's page size in Bytes. */
#define KERNEL_PAGE_SIZE 4096

/** @brief Page directory flag: 4Kb page size. */
#define PG_DIR_FLAG_PAGE_SIZE_4KB       0x00000000
/** @brief Page directory flag: 4Mb page size. */
#define PG_DIR_FLAG_PAGE_SIZE_4MB       0x00000080
/** @brief Page directory flag: page accessed. */
#define PG_DIR_FLAG_PAGE_ACCESSED       0x00000020
/** @brief Page directory flag: cache disabled. */
#define PG_DIR_FLAG_PAGE_CACHE_DISABLED 0x00000010
/** @brief Page directory flag: cache write policy set to write through. */
#define PG_DIR_FLAG_PAGE_CACHE_WT       0x00000008
/** @brief Page directory flag: cache write policy set to write back. */
#define PG_DIR_FLAG_PAGE_CACHE_WB       0x00000000
/** @brief Page directory flag: access permission set to user. */
#define PG_DIR_FLAG_PAGE_USER_ACCESS    0x00000004
/** @brief Page directory flag: access permission set to kernel. */
#define PG_DIR_FLAG_PAGE_SUPER_ACCESS   0x00000000
/** @brief Page directory flag: access permission set to read and write. */
#define PG_DIR_FLAG_PAGE_READ_WRITE     0x00000002
/** @brief Page directory flag: access permission set to read only. */
#define PG_DIR_FLAG_PAGE_READ_ONLY      0x00000000
/** @brief Page directory flag: page table present. */
#define PG_DIR_FLAG_PAGE_PRESENT        0x00000001
/** @brief Page directory flag: page table not present. */
#define PG_DIR_FLAG_PAGE_NOT_PRESENT    0x00000000

/** @brief Page flag: global page. */
#define PAGE_FLAG_GLOBAL         0x00000100
/** @brief Page flag: page dirty. */
#define PAGE_FLAG_DIRTY          0x00000080
/** @brief Page flag: page accessed. */
#define PAGE_FLAG_ACCESSED       0x00000020
/** @brief Page flag: cache disabled for the page. */
#define PAGE_FLAG_CACHE_DISABLED 0x00000010
/** @brief Page flag: cache write policy set to write through. */
#define PAGE_FLAG_CACHE_WT       0x00000008
/** @brief Page flag: cache write policy set to write back. */
#define PAGE_FLAG_CACHE_WB       0x00000000
/** @brief Page flag: access permission set to user. */
#define PAGE_FLAG_USER_ACCESS    0x00000004
/** @brief Page flag: access permission set to kernel. */
#define PAGE_FLAG_SUPER_ACCESS   0x00000000
/** @brief Page flag: access permission set to read and write. */
#define PAGE_FLAG_READ_WRITE     0x00000002
/** @brief Page flag: access permission set to read only. */
#define PAGE_FLAG_READ_ONLY      0x00000000
/** @brief Page flag: page present. */
#define PAGE_FLAG_PRESENT        0x00000001
/** @brief Page flag: page not present. */
#define PAGE_FLAG_NOT_PRESENT    0x00000000

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initializes paging structures for the kernel.
 *
 * @details Initializes paging structures for the kernel. This function will
 * select an available memory region to allocate the memory required for the
 * kernel. Then the kernel will be mapped to memory and paging is enabled for
 * the kernel.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NO_MORE_FREE_MEM is returned if there is not enough memory available
 *   to map the kernel.
 */
OS_RETURN_E paging_init(void);

/**
 * @brief Enables paging.
 *
 * @details Enables paging. The CR0 register is modified to enable paging.
 *
 * @warning CR3 register must be set before calling this function.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_PAGING_NOT_INIT is returned if paging has not been initialized.
 */
OS_RETURN_E paging_enable(void);

/**
 * @brief Disables paging.
 *
 * @details Disables paging. The CR0 register is modified to disable paging.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_PAGING_NOT_INIT is returned if paging has not been initialized.
 */
OS_RETURN_E paging_disable(void);

/**
 * @brief Maps a kernel virtual memory region to a corresponding physical
 * region.
 *
 * @details Maps a kernel virtual memory region to a corresponding physical
 * region. The function will not check any address boundaries. If the page is
 * already mapped and the allow_remap argument is set to 0 an error will be
 * returned.
 *
 * @warning The physical address is not checked for presence. IT will be aligned
 * to page boundaries.
 *
 * @param[in] virt_addr The virtual address to map.
 * @param[in] phys_addr  The physical address to be mapped.
 * @param[in] mapping_size The size of the region to map.
 * @param[in] flags The flags to be set to the pages created.
 * @param[in] allow_remap If set to 1, if the page is already mapped, this
 * mapping will  be replaced. If set to 0, if the page is already mapped, an
 * error is returned.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_PAGING_NOT_INIT is returned if paging has not been initialized.
 * - OS_ERR_MAPPING_ALREADY_EXISTS is returned if the page is already mapped.
 */
OS_RETURN_E kernel_direct_mmap(const void* virt_addr, const void* phys_addr,
                               const uint32_t mapping_size,
                               const uint16_t flags,
                               const uint16_t allow_remap);

/**
 * @brief Maps a kernel virtual memory region to a free physical region.
 *
 * @details Maps a kernel virtual memory region to a free physical
 * region. The function will not check any address boundaries. If the page is
 * already mapped and the allow_remap argument is set to 0 an error will be
 * returned.
 *
 * @param[in] virt_addr The virtual address to map.
 * @param[in] mapping_size The size of the region to map.
 * @param[in] flags The flags to be set to the pages created.
 * @param[in] allow_remap If set to 1, if the page is already mapped, this
 * mapping will  be replaced. If set to 0, if the page is already mapped, an
 * error is returned.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_PAGING_NOT_INIT is returned if paging has not been initialized.
 * - OS_ERR_MAPPING_ALREADY_EXISTS is returned if the page is already mapped.
 */
OS_RETURN_E kernel_mmap(const void* virt_addr, const uint32_t mapping_size,
                        const uint16_t flags, const uint16_t allow_remap);
/**
 * @brief Un-maps a kernel virtual memory region from a corresponding physical
 * region.
 *
 * @details Un-maps a kernel virtual memory region from a corresponding physical
 * region.
 *
 * @param[in] virt_addr The virtual address to map.
 * @param[in] mapping_size The size of the region to map.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_PAGING_NOT_INIT is returned if paging has not been initialized.
 * - OS_ERR_MEMORY_NOT_MAPPED is returned if the page is not mapped.
 */
OS_RETURN_E kernel_munmap(const void* virt_addr, const uint32_t mapping_size);

#endif /* __PAGING_H_ */