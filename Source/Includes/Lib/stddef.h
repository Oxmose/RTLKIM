/***************************************************************************//**
 * @file stddef.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 04/10/2017
 *
 * @version 1.0
 *
 * @brief Standard definitions for the kernel.
 * 
 * @details Standard definitions for the kernel. Contains the RTLK error codes
 * definition, and some types definitions.
 * 
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __STDDEF_H_
#define __STDDEF_H_

#include <IO/kernel_output.h> /* kernel_panic(), kernel_error() */
#include <Lib/stdint.h>       /* Generic int types */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief RTLK's NULL definition. */
#define NULL ((void *)0)

/** @brief RTLK's assert definition. */
#define assert(expr) \
    ((void)((expr) ? 0 : \
        (kernel_error(__FILE__":%u: failed assertion `"#expr"'\n", \
            __LINE__), 0))); kernel_panic(0)\

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/** @brief System return states enumeration. */
enum OS_RETURN
{
    /** @brief No error occured. */
    OS_NO_ERR                              = 0,
    /** @brief RTLK Error value. */
    OS_ERR_NULL_POINTER                    = 1,
    /** @brief RTLK Error value. */
    OS_ERR_OUT_OF_BOUND                    = 2,
    /** @brief RTLK Error value. */
    OR_ERR_UNAUTHORIZED_INTERRUPT_LINE     = 3,
    /** @brief RTLK Error value. */
    OS_ERR_INTERRUPT_ALREADY_REGISTERED    = 4,
    /** @brief RTLK Error value. */
    OS_ERR_INTERRUPT_NOT_REGISTERED        = 5,
    /** @brief RTLK Error value. */
    OS_ERR_NO_SUCH_IRQ_LINE                = 6,
    /** @brief RTLK Error value. */
    OS_ERR_NO_MORE_FREE_EVENT              = 7,
    /** @brief RTLK Error value. */
    OS_ERR_NO_SUCH_ID                      = 8,
    /** @brief RTLK Error value. */
    OS_ERR_MALLOC                          = 9,
    /** @brief RTLK Error value. */
    OS_ERR_UNAUTHORIZED_ACTION             = 10,
    /** @brief RTLK Error value. */
    OS_ERR_FORBIDEN_PRIORITY               = 11,
    /** @brief RTLK Error value. */
    OS_ERR_MUTEX_UNINITIALIZED             = 12,
    /** @brief RTLK Error value. */
    OS_ERR_SEM_UNINITIALIZED               = 13,
    /** @brief RTLK Error value. */
    OS_ERR_MAILBOX_NON_INITIALIZED         = 14,
    /** @brief RTLK Error value. */
    OS_ERR_QUEUE_NON_INITIALIZED           = 15,
    /** @brief RTLK Error value. */
    OS_ERR_NO_SEM_BLOCKED                  = 16,
    /** @brief RTLK Error value. */
    OS_ERR_NO_MUTEX_BLOCKED                = 17,
    /** @brief RTLK Error value. */
    OS_ERR_GRAPHIC_MODE_NOT_SUPPORTED      = 19,
    /** @brief RTLK Error value. */
    OS_MUTEX_LOCKED                        = 20,
    /** @brief RTLK Error value. */
    OS_SEM_LOCKED                          = 21,
    /** @brief RTLK Error value. */
    OS_ERR_CHECKSUM_FAILED                 = 22,
    /** @brief RTLK Error value. */
    OS_ERR_ACPI_UNSUPPORTED                = 23,
    /** @brief RTLK Error value. */
    OS_ACPI_NOT_INITIALIZED                = 24,
    /** @brief RTLK Error value. */
    OS_ERR_NO_SUCH_LAPIC_ID                = 25,
    /** @brief RTLK Error value. */
    OS_ERR_NO_SUCH_SERIAL_BAUDRATE         = 26,
    /** @brief RTLK Error value. */
    OS_ERR_NO_SUCH_SERIAL_PARITY           = 27,
    /** @brief RTLK Error value. */
    OS_ERR_ATA_DEVICE_NOT_PRESENT          = 28,
    /** @brief RTLK Error value. */
    OS_ERR_ATA_DEVICE_ERROR                = 29,
    /** @brief RTLK Error value. */
    OS_ERR_ATA_BAD_SECTOR_NUMBER           = 30,
    /** @brief RTLK Error value. */
    OS_ERR_ATA_SIZE_TO_HUGE                = 31,
    /** @brief RTLK Error value. */
    OS_ERR_VESA_NOT_SUPPORTED              = 32,
    /** @brief RTLK Error value. */
    OS_ERR_VESA_MODE_NOT_SUPPORTED         = 33,
    /** @brief RTLK Error value. */
    OS_ERR_VESA_NOT_INIT                   = 34,
    /** @brief RTLK Error value. */
    OS_ERR_NO_MORE_FREE_MEM                = 35,
    /** @brief RTLK Error value. */
    OS_ERR_PAGING_NOT_INIT                 = 36,
    /** @brief RTLK Error value. */
    OS_ERR_MAPPING_ALREADY_EXISTS          = 37,
    /** @brief RTLK Error value. */
    OS_ERR_MEMORY_NOT_MAPPED               = 38,
    /** @brief RTLK Error value. */
    OS_ERR_SMBIOS_NOT_FOUND                = 39,
    /** @brief RTLK Error value. */
    OS_ERR_BAD_HANDLER                     = 40,
    /** @brief RTLK Error value. */
    OS_ERR_MBR_PARTITION_INDEX_TOO_LARGE   = 41,
    /** @brief RTLK Error value. */
    OS_ERR_BAD_PARTITION_FORMAT            = 42,
    /** @brief RTLK Error value. */
    OS_ERR_PART_ALREADY_MOUNTED            = 43,
    /** @brief RTLK Error value. */
    OS_ERR_PART_NOT_MOUNTED                = 44,
    /** @brief RTLK Error value. */
    OS_ERR_MOUNT_POINT_USED                = 45,
    /** @brief RTLK Error value. */
    OS_ERR_WRONG_MOUNT_POINT               = 46,
    /** @brief RTLK Error value. */
    OS_ERR_UNSUPPORTED_DEVICE_TYPE         = 47,
    /** @brief RTLK Error value. */
    OS_ERR_WRONG_FAT32_BPB                 = 48,
    /** @brief RTLK Error value. */
    OS_ERR_WRONG_FILESYSTEM                = 49,
    /** @brief RTLK Error value. */
    OS_ERR_FAT32_BPS_NOT_SUPPORTED         = 50,
    /** @brief RTLK Error value. */
    OS_ERR_FAT32_REQ_TOO_BIG               = 51,
    /** @brief RTLK Error value. */
    OS_ERR_NOT_A_FOLDER                    = 52,
    /** @brief RTLK Error value. */
    OS_ERR_FILE_NOT_FOUND                  = 53,
    /** @brief RTLK Error value. */
    OS_ERR_NOT_A_FILE                      = 54,
    /** @brief RTLK Error value. */
    OS_ERR_FILE_ALREADY_EXISTS             = 55,
    /** @brief RTLK Error value. */
    OS_ERR_BAD_CLUSTER                     = 56,
    /** @brief RTLK Error value. */
    OS_ERR_BAD_FILE_NAME                   = 57,
    /** @brief RTLK Error value. */
    OS_ERR_PERMISSION_DENIED               = 58
    /** @brief RTLK Error value. */
};

/** 
 * @brief Defines OS_RETURN_E type as a shorcut for enum OS_RETURN.
 */
typedef enum OS_RETURN OS_RETURN_E;

/** 
 * @brief Defines OS_EVENT_ID type as a renaming for int32_t.
 */
typedef int32_t OS_EVENT_ID;

#ifndef __SIZE_TYPE__
#error __SIZE_TYPE__ not defined
#endif

/** 
 * @brief Defines size_t type as a renaming for __SIZE_TYPE__.
 */
typedef __SIZE_TYPE__ size_t;

#ifndef __PTRDIFF_TYPE__
#error __PTRDIFF_TYPE__ not defined
#endif

/** 
 * @brief Defines ptrdiff_t type as a renaming for __PTRDIFF_TYPE__.
 */
typedef __PTRDIFF_TYPE__ ptrdiff_t;

/** 
 * @brief Defines intptr_t type as a renaming for int32_t.
 */
typedef int32_t intptr_t;

#endif /* __STDDEF_H_ */
