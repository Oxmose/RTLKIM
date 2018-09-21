/*******************************************************************************
 *
 * File: stddef.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 04/10/2017
 *
 * Version: 1.0
 *
 * Standard definitions for the kernel.
 ******************************************************************************/

#ifndef __STDDEF_H_
#define __STDDEF_H_

#include <Lib/stdint.h>                /* Generic int types */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/
#define NULL ((void *)0)

#ifdef NDEBUG

#define assert(expr) ((void)0)

#else
extern void kernel_panic(const uint32_t);
extern void kernel_error(const char*, ...);
#define assert(expr) \
    ((void)((expr) ? 0 : \
        (kernel_error(__FILE__":%u: failed assertion `"#expr"'\n", \
            __LINE__), 0))); kernel_panic(0)\

#endif

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/* System return states */
typedef enum
{
    OS_NO_ERR                              = 0,

    OS_ERR_NULL_POINTER                    = 1,
    OS_ERR_OUT_OF_BOUND                    = 2,

    OR_ERR_UNAUTHORIZED_INTERRUPT_LINE     = 3,
    OS_ERR_INTERRUPT_ALREADY_REGISTERED    = 4,
    OS_ERR_INTERRUPT_NOT_REGISTERED        = 5,

    OS_ERR_NO_SUCH_IRQ_LINE                = 6,

    OS_ERR_NO_MORE_FREE_EVENT              = 7,
    OS_ERR_NO_SUCH_ID                      = 8,
    OS_ERR_MALLOC                          = 9,
    OS_ERR_UNAUTHORIZED_ACTION             = 10,
    OS_ERR_FORBIDEN_PRIORITY               = 11,

    OS_ERR_MUTEX_UNINITIALIZED             = 12,
    OS_ERR_SEM_UNINITIALIZED               = 13,
    OS_ERR_MAILBOX_NON_INITIALIZED         = 14,
    OS_ERR_QUEUE_NON_INITIALIZED           = 15,

    OS_ERR_NO_SEM_BLOCKED                  = 16,
    OS_ERR_NO_MUTEX_BLOCKED                = 17,

    OS_ERR_GRAPHIC_MODE_NOT_SUPPORTED      = 19,

    OS_MUTEX_LOCKED                        = 20,
    OS_SEM_LOCKED                          = 21,

    OS_ERR_CHECKSUM_FAILED                 = 22,
    OS_ERR_ACPI_UNSUPPORTED                = 23,
    OS_ACPI_NOT_INITIALIZED                = 24,
    OS_ERR_NO_SUCH_LAPIC_ID                = 25,

    OS_ERR_NO_SUCH_SERIAL_BAUDRATE         = 26,
    OS_ERR_NO_SUCH_SERIAL_PARITY           = 27,

    OS_ERR_ATA_DEVICE_NOT_PRESENT          = 28,
    OS_ERR_ATA_DEVICE_ERROR                = 29,
    OS_ERR_ATA_BAD_SECTOR_NUMBER           = 30,
    OS_ERR_ATA_SIZE_TO_HUGE                = 31,

    OS_ERR_VESA_NOT_SUPPORTED              = 32,
    OS_ERR_VESA_MODE_NOT_SUPPORTED         = 33,
    OS_ERR_VESA_NOT_INIT                   = 34,

    OS_ERR_NO_MORE_FREE_MEM                = 35,
    OS_ERR_PAGING_NOT_INIT                 = 36,
    OS_ERR_MAPPING_ALREADY_EXISTS          = 37,
    OS_ERR_MEMORY_NOT_MAPPED               = 38,
    OS_ERR_SMBIOS_NOT_FOUND                = 39,

    OS_ERR_BAD_HANDLER                     = 40,

    OS_ERR_MBR_PARTITION_INDEX_TOO_LARGE   = 41,

    OS_ERR_BAD_PARTITION_FORMAT            = 42,
    OS_ERR_PART_ALREADY_MOUNTED            = 43,
    OS_ERR_PART_NOT_MOUNTED                = 44,
    OS_ERR_MOUNT_POINT_USED                = 45,
    OS_ERR_WRONG_MOUNT_POINT               = 46,
    OS_ERR_UNSUPPORTED_DEVICE_TYPE         = 47,
    OS_ERR_WRONG_FAT32_BPB                 = 48,
    OS_ERR_WRONG_FILESYSTEM                = 49,
    OS_ERR_FAT32_BPS_NOT_SUPPORTED         = 50,
    OS_ERR_FAT32_REQ_TOO_BIG               = 51,
    OS_ERR_NOT_A_FOLDER                    = 52,
    OS_ERR_FILE_NOT_FOUND                  = 53,
    OS_ERR_NOT_A_FILE                      = 54,
    OS_ERR_FILE_ALREADY_EXISTS             = 55,
    OS_ERR_BAD_CLUSTER                     = 56,
    OS_ERR_BAD_FILE_NAME                   = 57,
    OS_ERR_PERMISSION_DENIED               = 58
} OS_RETURN_E;

typedef int32_t OS_EVENT_ID;

#ifndef __SIZE_TYPE__
#error __SIZE_TYPE__ not defined
#endif

typedef __SIZE_TYPE__ size_t;

#ifndef __PTRDIFF_TYPE__
#error __PTRDIFF_TYPE__ not defined
#endif

typedef __PTRDIFF_TYPE__ ptrdiff_t;

typedef int32_t intptr_t;

#define MIN(v0, v1) ((v0 < v1) ? v0 : v1)

#endif /* __STDDEF_H_ */
