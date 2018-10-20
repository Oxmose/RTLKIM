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
#include <Interrupt/panic.h>  /* kernel_panic.h */

/* RTLK configuration file */
#include <config.h>

/* Header file */
#include <Memory/meminfo.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/* Memory map data */
/** @brief Memory map structure's size. */
extern uint32_t          memory_map_size;

/** @brief Memory map storage as an array of range. */
extern mem_range_t       memory_map_data[];

/** @brief Multiboot memory pointer, fileld by the bootloader. */
extern multiboot_info_t* multiboot_data_ptr;

/** @brief Kernel's static start symbol. */
extern uint8_t _start;

/** @brief Kernel's static end symbol. */
extern uint8_t _end;

/** @brief Kernel's limit adderss. */
extern uint8_t _kernel_end;

/* Heap position in memory */
/** @brief Kernel's heap start address. */
extern uint8_t kernel_heap_start;
/** @brief Kernel's heap limit adderss. */
extern uint8_t kernel_heap_end;

/** @brief Total ammount of memory in the system. */
static uint32_t total_memory;

/** @brief Static memory used by the kernel. */
static uint32_t static_used_memory;

/** @brief Kernel's heap used memory data. */
extern uint32_t kheap_mem_used;

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
    mmap = (multiboot_memory_map_t*)(multiboot_data_ptr->mmap_addr +
                                     KERNEL_MEM_OFFSET);
    mmap_end = (multiboot_memory_map_t*)((uint32_t)mmap +
                                         multiboot_data_ptr->mmap_length);
    i = 0;
    total_memory = 0;
    /* Mark all static used memory as used */
    static_used_memory = (uint32_t)&_end - (uint32_t)&_start;
    while(mmap < mmap_end)
    {
        total_memory += (uint32_t)mmap->len;

        memory_map_data[i].base  = (uint32_t)mmap->addr;
        memory_map_data[i].limit = (uint32_t)mmap->addr + (uint32_t)mmap->len;
        memory_map_data[i].type  = mmap->type;

        /* Adds all unsusable memory after the kernel's static end as used */
        if(mmap->type != 1 && (uint32_t)mmap->addr > (uint32_t)&_end)
        {
            static_used_memory += (uint32_t)mmap->len;
        }

        ++i;
        mmap = (multiboot_memory_map_t*)
               ((uint32_t)mmap + mmap->size + sizeof(mmap->size));
    }
    memory_map_size = i;

    kernel_info("Memory map: \n");
    for(i = 0; i < memory_map_size; ++i)
    {
        kernel_info("    Base 0x%08x, Limit 0x%08x, Type %02d, Length %uKB\n",
                    memory_map_data[i].base,
                    memory_map_data[i].limit,
                    memory_map_data[i].type,
                    (memory_map_data[i].limit - memory_map_data[i].base) / 1024
                    );
    }
    free_size   = (uint32_t)&kernel_heap_end - (uint32_t)&kernel_heap_start;
    static_free = (uint32_t)&kernel_heap_start - (uint32_t)&_end;
    size        = (uint32_t)&_end - (uint32_t)&_start;

    if((uint32_t)&_end >(uint32_t)&kernel_heap_start)
    {
        kernel_error("Error, kernel size if too big (%d), consider modifying "
                     "the configuration file.\n", (uint32_t)&_end );
        kernel_panic(OS_ERR_UNAUTHORIZED_ACTION);
    }

    kernel_info("Kernel memory ranges:\n\t[STATIC: 0x%08x - 0x%08x]\n"
                 "\t[DYNAMIC: 0x%08x - 0x%08x]\n",
                (uint32_t)&_start, (uint32_t)&_end,
                (uint32_t)&kernel_heap_start, (uint32_t)&kernel_heap_end);
    kernel_info("Kernel static size %uKb | %uMb\n", size / 1024,
                size / 1024 / 1024);
    kernel_info("Kernel free static memory %uKb | %uMb\n", static_free / 1024,
                static_free / 1024 / 1024);
    kernel_info("Kernel free dynamic memory %uKb | %uMb\n", free_size / 1024,
                free_size / 1024 / 1024);

    kernel_info("Total memory: %uKb | %uMb\n", total_memory / 1024,
                total_memory / 1024 / 1024);
    kernel_info("Used memory: %uKb | %uMb\n", static_used_memory / 1024,
                static_used_memory / 1024 / 1024);

    return OS_NO_ERR;
}

uint32_t meminfo_kernel_heap_usage(void)
{
    return kheap_mem_used;
}

uint32_t meminfo_kernel_heap_size(void)
{
    return (uint32_t)&kernel_heap_end - (uint32_t)&kernel_heap_start;
}

uint32_t meminfo_kernel_memory_usage(void)
{
    return static_used_memory + meminfo_kernel_heap_usage();
}

uint32_t meminfo_kernel_total_size(void)
{
    return (uint32_t)&_kernel_end - (uint32_t)&_start;
}

uint32_t meminfo_get_memory_size(void)
{
    return total_memory;
}