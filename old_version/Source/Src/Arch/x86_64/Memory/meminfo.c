/***************************************************************************//**
 * @file meminfo.c
 *
 * @see meminfo.h
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

#include <Lib/stdint.h>       /* Generic int types */
#include <Lib/stddef.h>       /* OS_RETURN_E */
#include <Boot/multiboot.h>   /* MULTIBOOT_MEMORY_AVAILABLE */
#include <IO/kernel_output.h> /* kernel_info */
#include <Cpu/panic.h>  /* kernel_panic.h */

/* UTK configuration file */
#include <config.h>

/* Header file */
#include <Memory/meminfo.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/** @brief Memory map structure's size. */
uint32_t          memory_map_size;

/** @brief Memory map storage as an array of range. */
mem_range_t       memory_map_data[100];

/** @brief Multiboot memory pointer, fileld by the bootloader. */
multiboot_info_t* multiboot_data_ptr;

/** @brief Kernel's static start symbol. */
extern uint8_t _start;

/** @brief Kernel's static end symbol. */
extern uint8_t _end;

/** @brief Kernel's static memory limit. */
extern uint8_t kernel_static_limit;

/** @brief Kernel's limit adderss. */
extern uint8_t _kernel_end;

/* Heap position in memory */
/** @brief Kernel's heap start address. */
extern uint8_t kernel_heap_start;
/** @brief Kernel's heap limit adderss. */
extern uint8_t kernel_heap_end;

/** @brief Total ammount of memory in the system. */
static uint64_t total_memory;

/** @brief Static memory used by the kernel. */
static uint64_t static_used_memory;

/** @brief Kernel's heap used memory data. */
extern uint64_t kheap_mem_used;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

OS_RETURN_E memory_map_init(void)
{
    multiboot_memory_map_t* mmap;
    multiboot_memory_map_t* mmap_end;
    uint32_t i;
    uint32_t free_size;
    uint32_t static_free;
    uint32_t size;

    /* Update memory poisition */
    multiboot_data_ptr = (multiboot_info_t*)
                            ((uint8_t*)multiboot_data_ptr + KERNEL_MEM_OFFSET);

    /* Copy multiboot data in upper memory */
    mmap = (multiboot_memory_map_t*)(address_t)(multiboot_data_ptr->mmap_addr +
                                     KERNEL_MEM_OFFSET);
    mmap_end = (multiboot_memory_map_t*)((address_t)mmap +
                                         multiboot_data_ptr->mmap_length);
    i = 0;
    /* Mark all static used memory as used */
    static_used_memory = (address_t)&_end - (address_t)&_start;
    while(mmap < mmap_end)
    {
        total_memory += mmap->len;

        memory_map_data[i].base  = (address_t)mmap->addr;
        memory_map_data[i].limit = (address_t)mmap->addr + (address_t)mmap->len;
        memory_map_data[i].type  = mmap->type;

        /* Adds all unsusable memory after the kernel's static end as used */
        if(mmap->type != 1 && (address_t)mmap->addr > (address_t)&_end)
        {
            static_used_memory += mmap->len;
        }

        ++i;
        mmap = (multiboot_memory_map_t*)
               ((address_t)mmap + mmap->size + sizeof(mmap->size));
    }
    memory_map_size = i;


    kernel_info("Memory map: \n");
    for(i = 0; i < memory_map_size; ++i)
    {
        kernel_info("Area 0x%p -> 0x%p | %02d | %17lluKB\n",
                    memory_map_data[i].base,
                    memory_map_data[i].limit,
                    memory_map_data[i].type,
                    (address_t)(memory_map_data[i].limit - memory_map_data[i].base) >> 10
                    );
    }
    free_size   = (address_t)&kernel_heap_end - (address_t)&kernel_heap_start;
    static_free = (address_t)&kernel_heap_start - (address_t)&_end;
    size        = (address_t)&_end - (address_t)&_start;

    kernel_info("Kernel memory ranges:\n");
    kernel_info("    [STATIC:  0x%p - 0x%p] \n\t"
                "   %lluKb (%lluKb used, %lluKb free)\n",
                &_start, &_end, 
                ((address_t)&kernel_heap_start - (address_t)&_start) >> 10, 
                size >> 10,
                static_free >> 10);
    kernel_info("    [DYNAMIC: 0x%p - 0x%p] \n\t"
                "   %lluKb (%lluKb used, %lluKb free)\n",
                &kernel_heap_start, &kernel_heap_end, 
                free_size >> 10, 0, free_size >> 10);

    kernel_info("Total memory: %lluKb | %lluMb\n", total_memory >> 10,
                total_memory >> 20);
    kernel_info("Used memory: %lluKb | %lluMb\n", static_used_memory >> 10,
                static_used_memory >> 20);

    if((address_t)&_end > (address_t)&kernel_static_limit)
    {
        kernel_error("Error, kernel size if too big (%u), consider modifying "
                     "the configuration file.\n", &_end );
        kernel_panic(OS_ERR_UNAUTHORIZED_ACTION);
    }

    return OS_NO_ERR;
}

uint64_t meminfo_kernel_heap_usage(void)
{
    return kheap_mem_used;
}

uint64_t meminfo_kernel_heap_size(void)
{
    return (address_t)&kernel_heap_end - (address_t)&kernel_heap_start;
}

uint64_t meminfo_kernel_memory_usage(void)
{
    return static_used_memory + meminfo_kernel_heap_usage();
}

uint64_t meminfo_kernel_total_size(void)
{
    return (address_t)&_kernel_end - KERNEL_MEM_OFFSET;
}

uint64_t meminfo_get_memory_size(void)
{
    return total_memory;
}