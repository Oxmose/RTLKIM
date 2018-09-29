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

/* Heap position in memory */
/** @brief Kernel's heap start address. */
extern uint8_t kernel_heap_start;
/** @brief Kernel's heap limit adderss. */
extern uint8_t kernel_heap_end;

static uint32_t total_memory;
static uint32_t static_used_memory;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

OS_RETURN_E memory_map_init(void)
{
    multiboot_memory_map_t* mmap;
    uint32_t i;
    uint32_t free_size;
    uint32_t static_free;
    uint32_t size;

    /* Copy multiboot data in upper memory */
    mmap = (multiboot_memory_map_t*)multiboot_data_ptr->mmap_addr;
    i = 0;
    total_memory = 0;
    /* Mark all static used memory as used */
    static_used_memory = (uint32_t)&_end;
    while((uint32_t)mmap < 
          multiboot_data_ptr->mmap_addr + multiboot_data_ptr->mmap_length)
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
    size        = (uint32_t)&_end;
    
    kernel_info("Kernel memory ranges:\n\t[STATIC: 0x%08x - 0x%08x]\n"
                 "\t[DYNAMIC: 0x%08x - 0x%08x]\n", 
                0, (uint32_t)&_end, 
                (uint32_t)&kernel_heap_start, (uint32_t)&kernel_heap_end );
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