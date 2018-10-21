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

#include <Lib/stddef.h>          /* OS_RETURN_E */
#include <Lib/stdint.h>          /* Generic int types */
#include <Memory/meminfo.h>      /* mem_range_t */
#include <Memory/paging_alloc.h> /* paging_alloc_init */
#include <Boot/multiboot.h>      /* MULTIBOOT_MEMORY_AVAILABLE */

/* RTLK configuration file */
#include <config.h>

/* Header file */
#include <Memory/paging.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/* Memory map data */
extern uint32_t    memory_map_size;
extern mem_range_t memory_map_data[];

/* Allocation tracking */
static mem_range_t current_mem_range;
static uint8_t*    next_free_frame;

static uint32_t kernel_pgdir[1024] __attribute__((aligned(4096)));
static uint32_t kernel_page_table[1024][1024] __attribute__((aligned(4096)));

static uint32_t init = 0;
static uint32_t enabled;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

__inline__ static void invalidate_tlb(void)
{
    /* Invalidate the TLB */
    __asm__ __volatile__("movl	%cr3,%eax");
	__asm__ __volatile__("movl	%eax,%cr3");
}

OS_RETURN_E paging_init(void)
{
    uint32_t i;
    uint32_t j;
    uint32_t kernel_memory_size;
    uint32_t dir_entry_count;
    uint32_t to_map;
    uint32_t start_entry;

    OS_RETURN_E err;

    /* Get the first range that is free */
    for(i = 0; i < memory_map_size; ++i)
    {
        if(memory_map_data[i].base != 0x00000000 &&
           memory_map_data[i].type == MULTIBOOT_MEMORY_AVAILABLE)
           {
               current_mem_range.base  = memory_map_data[i].base;
               current_mem_range.limit = memory_map_data[i].limit;
               current_mem_range.type  = memory_map_data[i].type;
               break;
           }
    }

    if(i == memory_map_size)
    {
        return OS_ERR_NO_MORE_FREE_MEM;
    }

    /* Init frame and page allocators */
    err = paging_alloc_init();

    if(err != OS_NO_ERR)
    {
        return err;
    }

    kernel_memory_size = meminfo_kernel_total_size() / KERNEL_PAGE_SIZE;
    if(meminfo_kernel_total_size() % KERNEL_PAGE_SIZE != 0)
    {
        ++kernel_memory_size;
    }

    #if PAGING_KERNEL_DEBUG == 1
    kernel_serial_debug("Kernel memory size: %u (%u pages)\n",
                        meminfo_kernel_total_size(),
                        kernel_memory_size);
    kernel_serial_debug("Selected free memory range: \n");
    kernel_serial_debug("\tBase 0x%08x, Limit 0x%08x, Length %uKB, Type %d\n",
                current_mem_range.base,
                current_mem_range.limit,
                (current_mem_range.limit - current_mem_range.base) / 1024,
                current_mem_range.type);
    #endif

    /* Init kernel pgdir */
    for(i = 0; i < 1024; ++i)
    {
        kernel_pgdir[i] = PG_DIR_FLAG_PAGE_SIZE_4KB |
                          PG_DIR_FLAG_PAGE_SUPER_ACCESS |
                          PG_DIR_FLAG_PAGE_READ_WRITE |
                          PG_DIR_FLAG_PAGE_NOT_PRESENT;
    }

    /* Map the  kernel, ID mapped for the kernel */
    dir_entry_count = kernel_memory_size / 1024;
    to_map = kernel_memory_size;
    start_entry = KERNEL_MEM_OFFSET / (4096 * 1024);
    if(kernel_memory_size % 1024 != 0)
    {
        ++dir_entry_count;
    }
    for(i = start_entry; i < dir_entry_count + start_entry; ++i)
    {
        for(j = 0; j < 1024 && to_map > 0; ++j)
        {
            kernel_page_table[i][j] = ((((i * 1024) + j) * 0x1000) -
                                        KERNEL_MEM_OFFSET) |
                                       PAGE_FLAG_SUPER_ACCESS |
                                       PAGE_FLAG_READ_WRITE |
                                       PAGE_FLAG_PRESENT;
            --to_map;
        }
        kernel_pgdir[i] = ((uint32_t)kernel_page_table[i] - KERNEL_MEM_OFFSET) |
                          PG_DIR_FLAG_PAGE_SIZE_4KB |
                          PG_DIR_FLAG_PAGE_SUPER_ACCESS |
                          PG_DIR_FLAG_PAGE_READ_WRITE |
                          PG_DIR_FLAG_PAGE_PRESENT;
    }

    /* First page is not mapped (catch NULL pointers) */
    kernel_page_table[0][0] = PAGE_FLAG_SUPER_ACCESS |
                               PAGE_FLAG_READ_WRITE |
                               PAGE_FLAG_NOT_PRESENT;
    kernel_pgdir[0] = ((uint32_t)kernel_page_table[0] - KERNEL_MEM_OFFSET) |
                          PG_DIR_FLAG_PAGE_SIZE_4KB |
                          PG_DIR_FLAG_PAGE_SUPER_ACCESS |
                          PG_DIR_FLAG_PAGE_READ_WRITE |
                          PG_DIR_FLAG_PAGE_PRESENT;
    /* Init next free frame */
    next_free_frame = (uint8_t*)((kernel_memory_size) * 0x1000);

    /* Check bounds */
    if((uint32_t)next_free_frame < current_mem_range.base ||
       (uint32_t)next_free_frame >= current_mem_range.limit)
    {
        kernel_error("Paging Out of Bounds: request=0x%08x, base=0x%08x, "
                     "limit=0x%08x\n",
                     next_free_frame, current_mem_range.base,
                     current_mem_range.limit);
        return OS_ERR_NO_MORE_FREE_MEM;
    }

    /* Set CR3 register */
    __asm__ __volatile__("push %eax");
    __asm__ __volatile__("push %ebp");
    __asm__ __volatile__("mov %esp, %ebp");
    __asm__ __volatile__("mov %%eax, %%cr3": :"a"((uint32_t)kernel_pgdir -
                                                  KERNEL_MEM_OFFSET));
    __asm__ __volatile__("mov %ebp, %esp");
    __asm__ __volatile__("pop %ebp");
    __asm__ __volatile__("pop %eax");

    #if PAGING_KERNEL_DEBUG == 1
    kernel_serial_debug("CR3 Set to 0x%08x \n", kernel_pgdir);
    #endif

    enabled = 0;
    init = 1;

    return paging_enable();
}

OS_RETURN_E paging_enable(void)
{
    if(init == 0)
    {
        return OS_ERR_PAGING_NOT_INIT;
    }

    if(enabled == 1)
    {
        return OS_NO_ERR;
    }

    /* Enable paging and write protect */
    __asm__ __volatile__("push %eax");
    __asm__ __volatile__("push %ebp");
    __asm__ __volatile__("mov %esp, %ebp");
    __asm__ __volatile__("mov %cr0, %eax");
    __asm__ __volatile__("or $0x80010000, %eax");
    __asm__ __volatile__("mov %eax, %cr0");
    __asm__ __volatile__("mov %ebp, %esp");
    __asm__ __volatile__("pop %ebp");
    __asm__ __volatile__("pop %eax");

    #if PAGING_KERNEL_DEBUG == 1
    kernel_serial_debug("Paging enabled\n");
    #endif

    enabled = 1;

    return OS_NO_ERR;
}

OS_RETURN_E paging_disable(void)
{
    if(init == 0)
    {
        return OS_ERR_PAGING_NOT_INIT;
    }

    if(enabled == 0)
    {
        return OS_NO_ERR;
    }

    /* Disable paging and write protect */
    __asm__ __volatile__("push %ebp");
    __asm__ __volatile__("mov %esp, %ebp");
    __asm__ __volatile__("mov %cr0, %eax");
    __asm__ __volatile__("and $0x7FF7FFFF, %eax");
    __asm__ __volatile__("mov %eax, %cr0");
    __asm__ __volatile__("mov %ebp, %esp");
    __asm__ __volatile__("pop %ebp");

    #if PAGING_KERNEL_DEBUG == 1
    kernel_serial_debug("Paging disabled\n");
    #endif

    enabled = 0;

    return OS_NO_ERR;
}

OS_RETURN_E kernel_direct_mmap(const void* virt_addr, const void* phys_addr,
                               const uint32_t mapping_size,
                               const uint16_t flags,
                               const uint16_t allow_remap)
{
    uint32_t  pgdir_entry;
    uint32_t  pgtable_entry;
    uint32_t* page_table;
    uint32_t* page_entry;
    uint32_t  end_map;
    uint32_t  i;

    #if PAGING_KERNEL_DEBUG == 1
    uint32_t virt_save;
    #endif

    if(init == 0)
    {
        return OS_ERR_PAGING_NOT_INIT;
    }

    /* Get end mapping addr */
    end_map = (uint32_t)virt_addr + mapping_size;

    #if PAGING_KERNEL_DEBUG == 1
    kernel_serial_debug("Mapping (before align) 0x%08x, to 0x%08x (%d bytes)\n",
                        virt_addr, phys_addr, mapping_size);
    #endif

    /* Align addr */
    virt_addr = (uint8_t*)((uint32_t)virt_addr & 0xFFFFF000);
    phys_addr = (uint8_t*)((uint32_t)phys_addr & 0xFFFFF000);

    #if PAGING_KERNEL_DEBUG == 1
    kernel_serial_debug("Mapping (after align) 0x%08x, to 0x%08x (%d bytes)\n",
                        virt_addr, phys_addr, mapping_size);
    virt_save = (uint32_t)virt_addr;
    #endif

    /* Map all pages needed */
    while((uint32_t)virt_addr < end_map)
    {
        /* Get PGDIR entry */
        pgdir_entry = (((uint32_t)virt_addr) >> 22);
        /* Get PGTABLE entry */
        pgtable_entry = (((uint32_t)virt_addr) >> 12) & 0x03FF;

        /* If page table not present createe it */
        if((kernel_pgdir[pgdir_entry] & PG_DIR_FLAG_PAGE_PRESENT) !=
           PG_DIR_FLAG_PAGE_PRESENT)
        {
            page_table = (uint32_t*)kernel_page_table[pgdir_entry];

            for(i = 0; i < 1024; ++i)
            {
                page_table[i] = PAGE_FLAG_SUPER_ACCESS |
                                PAGE_FLAG_READ_ONLY |
                                PAGE_FLAG_NOT_PRESENT;
            }

            kernel_pgdir[pgdir_entry] = ((uint32_t)page_table  -
                                         KERNEL_MEM_OFFSET) |
                                        PG_DIR_FLAG_PAGE_SIZE_4KB |
                                        PG_DIR_FLAG_PAGE_SUPER_ACCESS |
                                        PG_DIR_FLAG_PAGE_READ_WRITE |
                                        PG_DIR_FLAG_PAGE_PRESENT;
        }

        /* Map the address */
        page_table = (uint32_t*)kernel_page_table[pgdir_entry];
        page_entry = &page_table[pgtable_entry];

        /* Check if already mapped */
        if((*page_entry & PAGE_FLAG_PRESENT) == PAGE_FLAG_PRESENT &&
           allow_remap == 0)
        {
            #if PAGING_KERNEL_DEBUG == 1
            kernel_serial_debug("Mapping (after align) 0x%08x, to 0x%08x "
                                "(%d bytes) Already mapped)\n",
                                virt_addr, phys_addr, mapping_size);
            virt_save = (uint32_t)virt_addr;
            #endif

            return OS_ERR_MAPPING_ALREADY_EXISTS;
        }

        *page_entry = (uint32_t)phys_addr |
                      flags |
                      PAGE_FLAG_PRESENT;

        virt_addr += KERNEL_PAGE_SIZE;
        phys_addr += KERNEL_PAGE_SIZE;
    }

    #if PAGING_KERNEL_DEBUG == 1
    /* Get PGDIR entry */
    pgdir_entry = (((uint32_t)virt_save) >> 22);
    /* Get PGTABLE entry */
    pgtable_entry = (((uint32_t)virt_save) >> 12) & 0x03FF;

    kernel_serial_debug("Mapped 0x%08x -> 0x%08x\n", virt_save,
                        ((uint32_t*)kernel_page_table[pgdir_entry])
                        [pgtable_entry]);
    #endif

    invalidate_tlb();

    return OS_NO_ERR;
}

OS_RETURN_E kernel_mmap(const void* virt_addr, const uint32_t mapping_size,
                        const uint16_t flags, const uint16_t allow_remap)
{
    uint32_t    pgdir_entry;
    uint32_t    pgtable_entry;
    uint32_t*   page_table;
    uint32_t*   page_entry;
    uint32_t    end_map;
    uint32_t    i;
    uint32_t    virt_save;
    void*       phys_addr;
    OS_RETURN_E err;

    #if PAGING_KERNEL_DEBUG == 1

    #endif

    if(init == 0)
    {
        return OS_ERR_PAGING_NOT_INIT;
    }

    /* Get end mapping addr */
    end_map = (uint32_t)virt_addr + mapping_size;

    #if PAGING_KERNEL_DEBUG == 1
    kernel_serial_debug("Mapping (before align) 0x%08x(%d bytes)\n",
                        virt_addr, mapping_size);
    #endif

    /* Align addr */
    virt_addr = (uint8_t*)((uint32_t)virt_addr & 0xFFFFF000);

    #if PAGING_KERNEL_DEBUG == 1
    kernel_serial_debug("Mapping (after align) 0x%08x (%d bytes)\n",
                        virt_addr, mapping_size);
    #endif

    virt_save = (uint32_t)virt_addr;
    err = OS_NO_ERR;

    /* Map all pages needed */
    while((uint32_t)virt_addr < end_map)
    {
        /* Get PGDIR entry */
        pgdir_entry = (((uint32_t)virt_addr) >> 22);
        /* Get PGTABLE entry */
        pgtable_entry = (((uint32_t)virt_addr) >> 12) & 0x03FF;

        /* If page table not present createe it */
        if((kernel_pgdir[pgdir_entry] & PG_DIR_FLAG_PAGE_PRESENT) !=
           PG_DIR_FLAG_PAGE_PRESENT)
        {
            page_table = (uint32_t*)kernel_page_table[pgdir_entry];

            for(i = 0; i < 1024; ++i)
            {
                page_table[i] = PAGE_FLAG_SUPER_ACCESS |
                                PAGE_FLAG_READ_ONLY |
                                PAGE_FLAG_NOT_PRESENT;
            }

            kernel_pgdir[pgdir_entry] = ((uint32_t)page_table  -
                                         KERNEL_MEM_OFFSET) |
                                        PG_DIR_FLAG_PAGE_SIZE_4KB |
                                        PG_DIR_FLAG_PAGE_SUPER_ACCESS |
                                        PG_DIR_FLAG_PAGE_READ_WRITE |
                                        PG_DIR_FLAG_PAGE_PRESENT;
        }

        /* Map the address */
        page_table = (uint32_t*)kernel_page_table[pgdir_entry];
        page_entry = &page_table[pgtable_entry];

        /* Check if already mapped */
        if((*page_entry & PAGE_FLAG_PRESENT) == PAGE_FLAG_PRESENT &&
           allow_remap == 0)
        {
            #if PAGING_KERNEL_DEBUG == 1
            kernel_serial_debug("Mapping (after align) 0x%08x "
                                "(%d bytes) Already mapped\n",
                                virt_addr, mapping_size);
            #endif

            return OS_ERR_MAPPING_ALREADY_EXISTS;
        }

        /* Get a new frame */
        phys_addr = kernel_paging_alloc_frames(1, &err);
        if(phys_addr == NULL)
        {
            break;
        }

        *page_entry = (uint32_t)phys_addr |
                      flags |
                      PAGE_FLAG_PRESENT;

        virt_addr += KERNEL_PAGE_SIZE;
    }
    /* If there was an error, unmap all that we mapped */
    if(err != OS_NO_ERR)
    {
        while(virt_save < (uint32_t)virt_addr)
        {
            kernel_munmap((void*)virt_save, KERNEL_PAGE_SIZE);
            virt_save += KERNEL_PAGE_SIZE;
        }

        return err;
    }

    #if PAGING_KERNEL_DEBUG == 1
    /* Get PGDIR entry */
    pgdir_entry = (((uint32_t)virt_save) >> 22);
    /* Get PGTABLE entry */
    pgtable_entry = (((uint32_t)virt_save) >> 12) & 0x03FF;

    kernel_serial_debug("Mapped 0x%08x -> 0x%08x\n", virt_save,
                        ((uint32_t*)kernel_page_table[pgdir_entry])
                        [pgtable_entry]);
    #endif

    invalidate_tlb();

    return OS_NO_ERR;
}

OS_RETURN_E kernel_munmap(const void* virt_addr, const uint32_t mapping_size)
{
    uint32_t    pgdir_entry;
    uint32_t    pgtable_entry;
    uint32_t*   page_table;
    uint32_t*   page_entry;
    uint32_t    end_map;
    void*       phy_addr;

    #if PAGING_KERNEL_DEBUG == 1
    uint32_t virt_save;
    #endif

    if(init == 0)
    {
        return OS_ERR_PAGING_NOT_INIT;
    }

    /* Get end mapping addr */
    end_map = (uint32_t)virt_addr + mapping_size;

    #if PAGING_KERNEL_DEBUG == 1
    kernel_serial_debug("Unmapping (before align) 0x%08x (%d bytes)\n",
                        virt_addr, mapping_size);
    #endif

    /* Align addr */
    virt_addr = (uint8_t*)((uint32_t)virt_addr & 0xFFFFF000);

    #if PAGING_KERNEL_DEBUG == 1
    kernel_serial_debug("Unmapping (after align) 0x%08x (%d bytes)\n",
                        virt_addr, mapping_size);
    virt_save = (uint32_t)virt_addr;
    #endif

    /* Unmap all pages needed */
    while((uint32_t)virt_addr < end_map)
    {
        /* Get PGDIR entry */
        pgdir_entry = (((uint32_t)virt_addr) >> 22);
        /* Get PGTABLE entry */
        pgtable_entry = (((uint32_t)virt_addr) >> 12) & 0x03FF;

        /* If page table not present no need to unmap */
        if((kernel_pgdir[pgdir_entry] & PG_DIR_FLAG_PAGE_PRESENT) !=
           PG_DIR_FLAG_PAGE_PRESENT)
        {
            return OS_ERR_MEMORY_NOT_MAPPED;
        }

        /* Get the address */
        page_table = (uint32_t*)kernel_page_table[pgdir_entry];
        page_entry = &page_table[pgtable_entry];

        phy_addr = (void*)((uint32_t)page_entry & 0xFFFFF000);

        /* Release the frame */
        kernel_paging_free_frames(phy_addr, 1);

        if((*page_entry & PAGE_FLAG_PRESENT) !=
           PAGE_FLAG_PRESENT)
        {
            return OS_ERR_MEMORY_NOT_MAPPED;
        }

        *page_entry = PAGE_FLAG_SUPER_ACCESS |
                      PAGE_FLAG_READ_ONLY |
                      PAGE_FLAG_NOT_PRESENT;

        virt_addr += KERNEL_PAGE_SIZE;
    }

    #if PAGING_KERNEL_DEBUG == 1
    /* Get PGDIR entry */
    pgdir_entry = (((uint32_t)virt_save) >> 22);
    /* Get PGTABLE entry */
    pgtable_entry = (((uint32_t)virt_save) >> 12) & 0x03FF;

    kernel_serial_debug("Unmapped 0x%08x -> 0x%08x\n", virt_save,
                        ((uint32_t*)kernel_page_table[pgdir_entry])
                        [pgtable_entry]);
    #endif

    invalidate_tlb();

    return OS_NO_ERR;
}