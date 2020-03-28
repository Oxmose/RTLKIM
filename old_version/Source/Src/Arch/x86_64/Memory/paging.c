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
#include <Lib/string.h>          /* memset */
#include <Cpu/panic.h>           /* kernel_panic */
#include <Memory/meminfo.h>      /* mem_range_t */
#include <Memory/paging_alloc.h> /* paging_alloc_init */
#include <Boot/multiboot.h>      /* MULTIBOOT_MEMORY_AVAILABLE */
#include <Core/scheduler.h>      /* sched_get_thread_pgdir */
#include <Memory/arch_paging.h>  /* Paging information */

/* UTK configuration file */
#include <config.h>

/* Header file */
#include <Memory/paging.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

#define PG_TABLE_ENTRY_COUNT      512ULL
#define PG_TABLE_LAST_ENTRY       (PG_TABLE_ENTRY_COUNT - 1)
#define PG_TABLE_RECUR_ENTRY      (PG_TABLE_LAST_ENTRY - 1)
#define PG_TABLE_RECUR_ENTRY_ADDR (0xFFFF000000000000 |       \
                                   (PG_TABLE_LAST_ENTRY << PML4_OFFSET) |      \
                                   (PG_TABLE_LAST_ENTRY << PDPT_OFFSET) |      \
                                   (PG_TABLE_LAST_ENTRY << PDT_OFFSET)  |      \
                                   (PG_TABLE_RECUR_ENTRY << PT_OFFSET))    

#define KERNEL_MIN_PGTABLE_SIZE 128

#define INVAL_PAGE(virt_addr)            \
{                                        \
    __asm__ __volatile__(                \
        "invlpg (%0)": :"r"(virt_addr) : "memory"     \
    );                                   \
}

/* Memory map data */
extern uint32_t    memory_map_size;
extern mem_range_t memory_map_data[];

/* Allocation tracking */
static mem_range_t current_mem_range;
static uint8_t*    next_free_frame;

address_t kernel_curr_pml4[PG_TABLE_ENTRY_COUNT]__attribute__((aligned(4096)));

address_t secure_pages[KERNEL_MIN_PGTABLE_SIZE][PG_TABLE_ENTRY_COUNT]__attribute__((aligned(4096)));

uint32_t  last_min_page = 0;

static uint32_t init = 0;
static uint32_t enabled;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

__inline__ static void invalidate_tlb(void)
{
    address_t tmp;

    /* Invalidate the TLB */
    __asm__ __volatile__("mov %%cr3, %0\n\t"
	                     "mov %0, %%cr3" : "=r" (tmp));
}

static void* alloc_minpage()
{
    if(last_min_page > KERNEL_MIN_PGTABLE_SIZE)
    {
        kernel_error("Cannot allocate min page, increase " 
                     "KERNEL_MIN_PGTABLE_SIZE\n");
        kernel_panic(OS_ERR_NO_MORE_FREE_MEM);
    }
    return &secure_pages[last_min_page++];
}

static void paging_recur_init(void)
{
    /* PLM4 */
    kernel_curr_pml4[PG_TABLE_LAST_ENTRY] = 
                        ((address_t)kernel_curr_pml4 - KERNEL_MEM_OFFSET) |
                        PG_STRUCT_ATTR_KERNEL_ACCESS | 
                        PG_STRUCT_ATTR_READ_WRITE |
                        PG_STRUCT_ATTR_ENABLED_CACHE |
                        PG_STRUCT_ATTR_WB_CACHE |
                        PG_STRUCT_ATTR_PRESENT;
    #if PAGING_KERNEL_DEBUG == 1
    kernel_serial_debug("Enabled recursive paging\n");
    #endif 
}

static uint8_t is_mapped(const void* virt_addr,
                         const uint32_t mapping_size)
{
    uint16_t  pml4_current_entry;
    uint16_t  pdpt_current_entry;
    uint16_t  pdt_current_entry;
    uint16_t  pt_current_entry;

    uint64_t* new_pdpt;
    uint64_t* new_pdt;
    uint64_t* new_pt;

    uint64_t* recur_entry_addr;
    uint64_t recur_entry_val;

    uint64_t to_map;
    uint8_t is_mapped_ret;

    address_t curr_virt_addr = (address_t)virt_addr & 0xFFFFFFFFFFFFF000;

    to_map = mapping_size;
    is_mapped_ret = 0;
    recur_entry_addr = (uint64_t*)PG_TABLE_RECUR_ENTRY_ADDR;

    /* Save the recursion entry value */
    recur_entry_val = kernel_curr_pml4[PG_TABLE_RECUR_ENTRY];

    while(to_map)
    {
        /* Get table entries */
        pml4_current_entry = ((address_t)curr_virt_addr >> PML4_OFFSET) & 0x1FF;
        pdpt_current_entry = ((address_t)curr_virt_addr >> PDPT_OFFSET) & 0x1FF;
        pdt_current_entry  = ((address_t)curr_virt_addr >> PDT_OFFSET) & 0x1FF;
        pt_current_entry   = ((address_t)curr_virt_addr >> PT_OFFSET) & 0x1FF;

        #if PAGING_KERNEL_DEBUG == 1
        kernel_serial_debug("Check Mapping 0x%p to 0x%p "
                            "(PML %d, PDPT %d, PDT %d, PT %d)\n",
                            curr_virt_addr, curr_virt_addr + to_map,
                            pml4_current_entry, pdpt_current_entry,
                            pdt_current_entry, pt_current_entry);
        #endif 

        /* Check for mapping in PML4 */
        if((kernel_curr_pml4[pml4_current_entry] & PG_STRUCT_ATTR_PRESENT) != 
        PG_STRUCT_ATTR_PRESENT)
        {
            to_map -= MIN(to_map, ((address_t)1 << PML4_OFFSET));
            curr_virt_addr += ((address_t)1 << PML4_OFFSET);
            continue;
        }
        else 
        {
            new_pdpt = (uint64_t*)(kernel_curr_pml4[pml4_current_entry] & 
                                0xFFFFFFFFFFFFF000);
        }

        /* Recursive map the entry */
        kernel_curr_pml4[PG_TABLE_RECUR_ENTRY] = 
                        (address_t)new_pdpt |
                        PG_STRUCT_ATTR_4KB_PAGES |
                        PG_STRUCT_ATTR_KERNEL_ACCESS | 
                        PG_STRUCT_ATTR_READ_WRITE |
                        PG_STRUCT_ATTR_ENABLED_CACHE |
                        PG_STRUCT_ATTR_WB_CACHE |
                        PG_STRUCT_ATTR_PRESENT;
        INVAL_PAGE(recur_entry_addr);

        /* Check for mapping in PDPT */
        if((recur_entry_addr[pdpt_current_entry] & PG_STRUCT_ATTR_PRESENT) != 
        PG_STRUCT_ATTR_PRESENT)
        {
            to_map -= MIN(to_map, ((address_t)1 << PDPT_OFFSET));
            curr_virt_addr += ((address_t)1 << PDPT_OFFSET);
            continue;
        }
        else 
        {
            new_pdt = (uint64_t*)(recur_entry_addr[pdpt_current_entry] & 
                                0xFFFFFFFFFFFFF000);
        }

        /* Recursive map the entry */
        kernel_curr_pml4[PG_TABLE_RECUR_ENTRY] = 
                        (address_t)new_pdt |
                        PG_STRUCT_ATTR_4KB_PAGES |
                        PG_STRUCT_ATTR_KERNEL_ACCESS | 
                        PG_STRUCT_ATTR_READ_WRITE |
                        PG_STRUCT_ATTR_ENABLED_CACHE |
                        PG_STRUCT_ATTR_WB_CACHE |
                        PG_STRUCT_ATTR_PRESENT;
        INVAL_PAGE(recur_entry_addr);

        /* Check for mapping in PDT */
        if((recur_entry_addr[pdt_current_entry] & PG_STRUCT_ATTR_PRESENT) != 
        PG_STRUCT_ATTR_PRESENT)
        {
            to_map -= MIN(to_map, ((address_t)1 << PDT_OFFSET));
            curr_virt_addr += ((address_t)1 << PDT_OFFSET);
            continue;
        }
        else 
        {
            new_pt = (uint64_t*)(recur_entry_addr[pdt_current_entry] & 
                                0xFFFFFFFFFFFFF000);
        }

        /* Recursive map the entry */
        kernel_curr_pml4[PG_TABLE_RECUR_ENTRY] = 
                        (address_t)new_pt |
                        PG_STRUCT_ATTR_4KB_PAGES |
                        PG_STRUCT_ATTR_KERNEL_ACCESS | 
                        PG_STRUCT_ATTR_READ_WRITE |
                        PG_STRUCT_ATTR_ENABLED_CACHE |
                        PG_STRUCT_ATTR_WB_CACHE |
                        PG_STRUCT_ATTR_PRESENT;
        INVAL_PAGE(recur_entry_addr);
        
        /* Check for mapping in PT */
        if((recur_entry_addr[pt_current_entry] & PG_STRUCT_ATTR_PRESENT) != 
        PG_STRUCT_ATTR_PRESENT)
        {
            to_map -= MIN(to_map, ((address_t)1 << PT_OFFSET));
            curr_virt_addr += ((address_t)1 << PT_OFFSET);
            continue;
        }

        is_mapped_ret = 1;
        to_map = 0;
    }

    /* Restore initial recursion value */
    kernel_curr_pml4[PG_TABLE_RECUR_ENTRY] = recur_entry_val;

    INVAL_PAGE(recur_entry_addr);

    #if PAGING_KERNEL_DEBUG == 1
    kernel_serial_debug("Check Mapping 0x%p: %d\n",
                        curr_virt_addr, is_mapped_ret);
    #endif 

    return is_mapped_ret;
}

OS_RETURN_E paging_init(void)
{
    uint64_t  i;
    uint64_t  kernel_page_count;

    uint16_t  pml4_current_entry;
    uint16_t  pdpt_current_entry;
    uint16_t  pdt_current_entry;
    uint16_t  pt_current_entry;

    uint64_t*  pdpt_current_entry_addr;
    uint64_t*  pdt_current_entry_addr;
    uint64_t*  pt_current_entry_addr;

    address_t  current_addr;

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

    kernel_page_count = meminfo_kernel_total_size() / KERNEL_PAGE_SIZE;
    if(meminfo_kernel_total_size() % KERNEL_PAGE_SIZE != 0)
    {
        ++kernel_page_count;
    }

    #if PAGING_KERNEL_DEBUG == 1
    kernel_serial_debug("Kernel memory size: %lluKB (%lu pages)\n",
                        meminfo_kernel_total_size() >> 10,
                        kernel_page_count);
    kernel_serial_debug("Selected free memory range: \n");
    kernel_serial_debug("\tBase 0x%p, Limit 0x%p, Length %lluKB, Type %d\n",
                current_mem_range.base,
                current_mem_range.limit,
                (current_mem_range.limit - current_mem_range.base) / 1024,
                current_mem_range.type);
    #endif

    /* Init the PML4 */
    for(i = 0; i < PG_TABLE_ENTRY_COUNT; ++i)
    {
        kernel_curr_pml4[i] = PG_STRUCT_ATTR_KERNEL_ACCESS |
                          PG_STRUCT_ATTR_READ_ONLY |
                          PG_STRUCT_ATTR_NOT_PRESENT;
    }

    /* Get the first entry of the kernel */
    pml4_current_entry = ((address_t)KERNEL_MEM_OFFSET >> PML4_OFFSET) & 0x1FF;
    pdpt_current_entry = ((address_t)KERNEL_MEM_OFFSET >> PDPT_OFFSET) & 0x1FF;
    pdt_current_entry  = ((address_t)KERNEL_MEM_OFFSET >> PDT_OFFSET) & 0x1FF;
    pt_current_entry   = ((address_t)KERNEL_MEM_OFFSET >> PT_OFFSET) & 0x1FF;

    /* Create the first link */
    pdpt_current_entry_addr  = alloc_minpage();
    if(pdpt_current_entry_addr == NULL)
    {
        kernel_error("Could not allocate PDPT\n");
        kernel_panic(OS_ERR_MALLOC);
    }
    pdt_current_entry_addr  = alloc_minpage();
    if(pdt_current_entry_addr == NULL)
    {
        kernel_error("Could not allocate PDT\n");
        kernel_panic(OS_ERR_MALLOC);
    }
    pt_current_entry_addr   = alloc_minpage();
    if(pt_current_entry_addr == NULL)
    {
        kernel_error("Could not allocate PT\n");
        kernel_panic(OS_ERR_MALLOC);
    }
    kernel_curr_pml4[pml4_current_entry] = 
                        ((address_t)pdpt_current_entry_addr - KERNEL_MEM_OFFSET) |
                        PG_STRUCT_ATTR_KERNEL_ACCESS | 
                        PG_STRUCT_ATTR_READ_WRITE |
                        PG_STRUCT_ATTR_ENABLED_CACHE |
                        PG_STRUCT_ATTR_WB_CACHE |
                        PG_STRUCT_ATTR_PRESENT;
    pdpt_current_entry_addr[pdpt_current_entry] = 
                        ((address_t)pdt_current_entry_addr - KERNEL_MEM_OFFSET) |
                        PG_STRUCT_ATTR_KERNEL_ACCESS | 
                        PG_STRUCT_ATTR_READ_WRITE |
                        PG_STRUCT_ATTR_ENABLED_CACHE |
                        PG_STRUCT_ATTR_WB_CACHE |
                        PG_STRUCT_ATTR_PRESENT;
    pdt_current_entry_addr[pdt_current_entry] = 
                        ((address_t)pt_current_entry_addr - KERNEL_MEM_OFFSET) |
                        PG_STRUCT_ATTR_KERNEL_ACCESS | 
                        PG_STRUCT_ATTR_READ_WRITE |
                        PG_STRUCT_ATTR_ENABLED_CACHE |
                        PG_STRUCT_ATTR_WB_CACHE |
                        PG_STRUCT_ATTR_PRESENT;

    current_addr = 0;

    /* Map the kernel */
    while(kernel_page_count > 0)
    {
        /* Check if we have to create a new page table */
        if(pt_current_entry > PG_TABLE_LAST_ENTRY)
        {
            /* Check if we have to create a new page directory table */
            if(pdt_current_entry > PG_TABLE_LAST_ENTRY)
            {
                /* Check if we have to create a new page directory 
                 * pointer table */
                if(pdpt_current_entry > PG_TABLE_LAST_ENTRY)
                {
                    /* Allocated new entry */
                    pdpt_current_entry = 0;
                    pdpt_current_entry_addr = alloc_minpage();

                    if(pdpt_current_entry_addr == NULL)
                    {
                        kernel_error("Could not allocate PDPT\n");
                        kernel_panic(OS_ERR_MALLOC);
                    }

                    /* Init new entry */
                    memset(pdpt_current_entry_addr, 0, KERNEL_PAGE_SIZE);

                    /* Update the entry in PML4 */
                    ++pml4_current_entry;
                    kernel_curr_pml4[pml4_current_entry] = 
                        ((address_t)pdpt_current_entry_addr - KERNEL_MEM_OFFSET) |
                        PG_STRUCT_ATTR_KERNEL_ACCESS | 
                        PG_STRUCT_ATTR_READ_WRITE |
                        PG_STRUCT_ATTR_ENABLED_CACHE |
                        PG_STRUCT_ATTR_WB_CACHE |
                        PG_STRUCT_ATTR_PRESENT;
                }
                else 
                {
                    ++pdpt_current_entry;
                }

                /* Allocated new entry */
                pdt_current_entry = 0;
                pdt_current_entry_addr = alloc_minpage();

                if(pdt_current_entry_addr == NULL)
                {
                    kernel_error("Could not allocate PDT\n");
                    kernel_panic(OS_ERR_MALLOC);
                }

                /* Init new entry */
                memset(pdt_current_entry_addr, 0, KERNEL_PAGE_SIZE); 

                /* Update the entry in PDPT */  
                pdpt_current_entry_addr[pdpt_current_entry] = 
                    ((address_t)pdt_current_entry_addr - KERNEL_MEM_OFFSET) |
                    PG_STRUCT_ATTR_KERNEL_ACCESS | 
                    PG_STRUCT_ATTR_READ_WRITE |
                    PG_STRUCT_ATTR_ENABLED_CACHE |
                    PG_STRUCT_ATTR_WB_CACHE |
                    PG_STRUCT_ATTR_PRESENT;
            }
            else 
            {
                ++pdt_current_entry;
            }

            /* Allocated new entry */
            pt_current_entry = 0;
            pt_current_entry_addr = alloc_minpage();

            if(pt_current_entry_addr == NULL)
            {
                kernel_error("Could not allocate PT\n");
                kernel_panic(OS_ERR_MALLOC);
            }

            /* Init new entry */
            memset(pt_current_entry_addr, 0, KERNEL_PAGE_SIZE); 

            /* Update the entry in PDT */  
            pdt_current_entry_addr[pdt_current_entry] = 
                ((address_t)pt_current_entry_addr - KERNEL_MEM_OFFSET) |
                PG_STRUCT_ATTR_KERNEL_ACCESS | 
                PG_STRUCT_ATTR_READ_WRITE |
                PG_STRUCT_ATTR_ENABLED_CACHE |
                PG_STRUCT_ATTR_WB_CACHE |
                PG_STRUCT_ATTR_PRESENT;
        }

        /* Map the page in the table */
        pt_current_entry_addr[pt_current_entry] = 
            current_addr |
            PG_STRUCT_ATTR_4KB_PAGES |
            PG_STRUCT_ATTR_KERNEL_ACCESS | 
            PG_STRUCT_ATTR_READ_WRITE |
            PG_STRUCT_ATTR_ENABLED_CACHE |
            PG_STRUCT_ATTR_WB_CACHE |
            PG_STRUCT_ATTR_PRESENT;

        ++pt_current_entry;
        --kernel_page_count;
        current_addr += KERNEL_PAGE_SIZE;                                                 
    }

    #if PAGING_KERNEL_DEBUG == 1
    kernel_serial_debug("Mapped the kernel in secure paging area\n");
    #endif 

    /* Init recursive paging */
    paging_recur_init();

    /* Init next free frame */
    kernel_page_count = meminfo_kernel_total_size() / KERNEL_PAGE_SIZE;
    if(meminfo_kernel_total_size() % KERNEL_PAGE_SIZE != 0)
    {
        ++kernel_page_count;
    }
    next_free_frame = (uint8_t*)((kernel_page_count) * 0x1000);
    if((address_t)next_free_frame < current_mem_range.base ||
       (address_t)next_free_frame >= current_mem_range.limit)
    {
        kernel_error("Paging Out of Bounds: request=0x%p, base=0x%p, "
                     "limit=0x%p\n",
                     next_free_frame, current_mem_range.base,
                     current_mem_range.limit);
        return OS_ERR_NO_MORE_FREE_MEM;
    }

    #if PAGING_KERNEL_DEBUG == 1
    kernel_serial_debug("Next free frame at 0x%p\n", next_free_frame);
    #endif

    /* Set CR3 register */
    __asm__ __volatile__("mov %%rax, %%cr3": :"a"((address_t)kernel_curr_pml4 -
                                                  KERNEL_MEM_OFFSET));

    #if PAGING_KERNEL_DEBUG == 1
    kernel_serial_debug("CR3 Set to 0x%p \n", kernel_curr_pml4-
                                                  KERNEL_MEM_OFFSET);
    #endif

    enabled = 0;
    init = 1;

    err = paging_enable();
    return err;
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
    __asm__ __volatile__("mov %%cr0, %%eax\n\t"
                         "or $0x80010000, %%eax\n\t"
                         "mov %%eax, %%cr0" : : : "eax");

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
    __asm__ __volatile__("mov %%cr0, %%eax\n\t"
                         "and $0x7FF7FFFF, %%eax\n\t"
                         "mov %%eax, %%cr0" : : : "eax");

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
    uint16_t  pml4_current_entry;
    uint16_t  pdpt_current_entry;
    uint16_t  pdt_current_entry;
    uint16_t  pt_current_entry;

    uint64_t* new_pdpt;
    uint64_t* new_pdt;
    uint64_t* new_pt;

    uint64_t* recur_entry_addr;
    
    uint64_t recur_entry_val;

    address_t curr_virt_addr = (address_t)virt_addr & 0xFFFFFFFFFFFFF000;
    address_t curr_phys_addr = (address_t)phys_addr & 0xFFFFFFFFFFFFF000;

    uint32_t to_map;

    uint8_t is_alread_mapped;

    OS_RETURN_E err;

    if(init == 0)
    {
        return OS_ERR_PAGING_NOT_INIT;
    }

    recur_entry_addr = (uint64_t*)PG_TABLE_RECUR_ENTRY_ADDR;

    /* Save the recursion entry value */
    recur_entry_val = kernel_curr_pml4[PG_TABLE_RECUR_ENTRY];

    to_map = mapping_size & 0xFFFFF000;
    if(mapping_size % 0x1000 != 0)
    {
        to_map += 0x1000;
    }

    while(to_map)
    {
        /* Check if we are remapping an entry */
        is_alread_mapped = is_mapped((void*)curr_virt_addr, KERNEL_PAGE_SIZE);
        if(!allow_remap && is_alread_mapped)
        {
            curr_phys_addr += KERNEL_PAGE_SIZE;
            curr_virt_addr += KERNEL_PAGE_SIZE;

            to_map -= KERNEL_PAGE_SIZE;
            continue;
        }
        /* TODO If already mapped, unmap properly */

        /* Get table entries */
        pml4_current_entry = ((address_t)curr_virt_addr >> PML4_OFFSET) & 0x1FF;
        pdpt_current_entry = ((address_t)curr_virt_addr >> PDPT_OFFSET) & 0x1FF;
        pdt_current_entry  = ((address_t)curr_virt_addr >> PDT_OFFSET) & 0x1FF;
        pt_current_entry   = ((address_t)curr_virt_addr >> PT_OFFSET) & 0x1FF;

        #if PAGING_KERNEL_DEBUG == 1
        kernel_serial_debug("Mapping 0x%p to 0x%p "
                            "(PML %d, PDPT %d, PDT %d, PT %d)\n",
                            curr_virt_addr, curr_phys_addr,
                            pml4_current_entry, pdpt_current_entry,
                            pdt_current_entry, pt_current_entry);
        #endif 

        /* Check for mapping in PML4 */
        if((kernel_curr_pml4[pml4_current_entry] & PG_STRUCT_ATTR_PRESENT) != 
        PG_STRUCT_ATTR_PRESENT)
        {
            /* Get a new frame for the PDPT */
            new_pdpt = kernel_paging_alloc_frames(1, &err);
            if(err != OS_NO_ERR)
            {
                return err;
            }

            /* Create new mapping */
            kernel_curr_pml4[pml4_current_entry] = 
                        (address_t)new_pdpt |
                        PG_STRUCT_ATTR_KERNEL_ACCESS | 
                        PG_STRUCT_ATTR_READ_WRITE |
                        PG_STRUCT_ATTR_ENABLED_CACHE |
                        PG_STRUCT_ATTR_WB_CACHE |
                        PG_STRUCT_ATTR_PRESENT;
        }
        else 
        {
            new_pdpt = (uint64_t*)(kernel_curr_pml4[pml4_current_entry] & 
                                0xFFFFFFFFFFFFF000);
        }

        /* Recursive map the entry */
        kernel_curr_pml4[PG_TABLE_RECUR_ENTRY] = 
                        (address_t)new_pdpt |
                        PG_STRUCT_ATTR_4KB_PAGES |
                        PG_STRUCT_ATTR_KERNEL_ACCESS | 
                        PG_STRUCT_ATTR_READ_WRITE |
                        PG_STRUCT_ATTR_ENABLED_CACHE |
                        PG_STRUCT_ATTR_WB_CACHE |
                        PG_STRUCT_ATTR_PRESENT;
        INVAL_PAGE(recur_entry_addr);

        /* Check for mapping in PDPT */
        if((recur_entry_addr[pdpt_current_entry] & PG_STRUCT_ATTR_PRESENT) != 
        PG_STRUCT_ATTR_PRESENT)
        {
            /* Get a new frame for the PDT */
            new_pdt = kernel_paging_alloc_frames(1, &err);
            if(err != OS_NO_ERR)
            {
                /* Restore initial recursion value */
                kernel_curr_pml4[PG_TABLE_RECUR_ENTRY] = recur_entry_val;

                /* Free frames */
                kernel_paging_free_frames(new_pdt, 1);

                INVAL_PAGE(recur_entry_addr);

                return err;
            }

            /* Create new mapping */
            recur_entry_addr[pdpt_current_entry] = 
                        (address_t)new_pdt |
                        PG_STRUCT_ATTR_KERNEL_ACCESS | 
                        PG_STRUCT_ATTR_READ_WRITE |
                        PG_STRUCT_ATTR_ENABLED_CACHE |
                        PG_STRUCT_ATTR_WB_CACHE |
                        PG_STRUCT_ATTR_PRESENT;
        }
        else 
        {
            new_pdt = (uint64_t*)(recur_entry_addr[pdpt_current_entry] & 
                                0xFFFFFFFFFFFFF000);
        }

        /* Recursive map the entry */
        kernel_curr_pml4[PG_TABLE_RECUR_ENTRY] = 
                        (address_t)new_pdt |
                        PG_STRUCT_ATTR_4KB_PAGES |
                        PG_STRUCT_ATTR_KERNEL_ACCESS | 
                        PG_STRUCT_ATTR_READ_WRITE |
                        PG_STRUCT_ATTR_ENABLED_CACHE |
                        PG_STRUCT_ATTR_WB_CACHE |
                        PG_STRUCT_ATTR_PRESENT;
        INVAL_PAGE(recur_entry_addr);

        /* Check for mapping in PDT */
        if((recur_entry_addr[pdt_current_entry] & PG_STRUCT_ATTR_PRESENT) != 
        PG_STRUCT_ATTR_PRESENT)
        {
            /* Get a new frame for the PT */
            new_pt = kernel_paging_alloc_frames(1, &err);
            if(err != OS_NO_ERR)
            {
                /* Restore initial recursion value */
                kernel_curr_pml4[PG_TABLE_RECUR_ENTRY] = recur_entry_val;

                /* Free frames */
                kernel_paging_free_frames(new_pdpt, 1);
                kernel_paging_free_frames(new_pdt, 1);

                INVAL_PAGE(recur_entry_addr);

                return err;
            }

            /* Create new mapping */
            recur_entry_addr[pdt_current_entry] = 
                        (address_t)new_pt |
                        PG_STRUCT_ATTR_KERNEL_ACCESS | 
                        PG_STRUCT_ATTR_READ_WRITE |
                        PG_STRUCT_ATTR_ENABLED_CACHE |
                        PG_STRUCT_ATTR_WB_CACHE |
                        PG_STRUCT_ATTR_PRESENT;
        }
        else 
        {
            new_pt = (uint64_t*)(recur_entry_addr[pdt_current_entry] & 
                                0xFFFFFFFFFFFFF000);
        }

        /* Recursive map the entry */
        kernel_curr_pml4[PG_TABLE_RECUR_ENTRY] = 
                        (address_t)new_pt |
                        PG_STRUCT_ATTR_4KB_PAGES |
                        PG_STRUCT_ATTR_KERNEL_ACCESS | 
                        PG_STRUCT_ATTR_READ_WRITE |
                        PG_STRUCT_ATTR_ENABLED_CACHE |
                        PG_STRUCT_ATTR_WB_CACHE |
                        PG_STRUCT_ATTR_PRESENT;
        INVAL_PAGE(recur_entry_addr);
        /* Now our page table is recursively mapped, map the address */
    

        /* Map page */
        recur_entry_addr[pt_current_entry] = 
                        (address_t) curr_phys_addr |
                        flags |
                        PG_STRUCT_ATTR_PRESENT;
        INVAL_PAGE(curr_virt_addr);

        #if PAGING_KERNEL_DEBUG == 1
        kernel_serial_debug("Mapped 0x%p to 0x%p "
                            "(PML %d, PDPT %d, PDT %d, PT %d)\n",
                            curr_virt_addr, curr_phys_addr,
                            pml4_current_entry, pdpt_current_entry,
                            pdt_current_entry, pt_current_entry);
        #endif 

        curr_phys_addr += KERNEL_PAGE_SIZE;
        curr_virt_addr += KERNEL_PAGE_SIZE;

        to_map -= KERNEL_PAGE_SIZE;
    }

    /* Restore initial recursion value */
    kernel_curr_pml4[PG_TABLE_RECUR_ENTRY] = recur_entry_val;

    INVAL_PAGE(recur_entry_addr);
    INVAL_PAGE(virt_addr);

    return OS_NO_ERR;
}

OS_RETURN_E kernel_mmap(const void* virt_addr, const uint32_t mapping_size,
                        const uint16_t flags, const uint16_t allow_remap)
{
    uint32_t    end_map;
    uint64_t    virt_save;
    void*       phys_addr;
    OS_RETURN_E err;

    if(init == 0)
    {
        return OS_ERR_PAGING_NOT_INIT;
    }

    /* Get end mapping addr */
    end_map = (address_t)virt_addr + mapping_size;

    #if PAGING_KERNEL_DEBUG == 1
    kernel_serial_debug("Mapping (before align) 0x%08x(%d bytes)\n",
                        virt_addr, mapping_size);
    #endif

    /* Align addr */
    virt_addr = (uint8_t*)((address_t)virt_addr & 0xFFFFF000);

    #if PAGING_KERNEL_DEBUG == 1
    kernel_serial_debug("Mapping (after align) 0x%08x (%d bytes)\n",
                        virt_addr, mapping_size);
    #endif

    virt_save = (address_t)virt_addr;
    err = OS_NO_ERR;

    /* Map all pages needed */
    while((address_t)virt_addr < end_map)
    {
        /* Get a new frame */
        phys_addr = kernel_paging_alloc_frames(1, &err);
        if(phys_addr == NULL)
        {
            break;
        }

        err = kernel_direct_mmap(virt_addr, phys_addr, 1, flags, allow_remap);
        if(err != OS_NO_ERR)
        {
            break;
        }

        virt_addr = (uint8_t*)virt_addr + KERNEL_PAGE_SIZE;
    }
    /* If there was an error, unmap all that we mapped */
    if(err != OS_NO_ERR)
    {
        while(virt_save < (address_t)virt_addr)
        {
            kernel_munmap((void*)virt_save, KERNEL_PAGE_SIZE);
            virt_save += KERNEL_PAGE_SIZE;
        }

        return err;
    }

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
    uint32_t*   current_pgdir;
    uint32_t    i;
    uint32_t*   pgtable_mapped;

    #if PAGING_KERNEL_DEBUG == 1
    uint32_t virt_save;
    #endif

    if(init == 0)
    {
        return OS_ERR_PAGING_NOT_INIT;
    }

    current_pgdir = (uint32_t*)&kernel_curr_pml4;

    /* Get end mapping addr */
    end_map = (address_t)virt_addr + mapping_size;

    #if PAGING_KERNEL_DEBUG == 1
    kernel_serial_debug("Unmapping (before align) 0x%08x (%d bytes)\n",
                        virt_addr, mapping_size);
    #endif

    /* Align addr */
    virt_addr = (uint8_t*)((address_t)virt_addr & 0xFFFFF000);

    #if PAGING_KERNEL_DEBUG == 1
    kernel_serial_debug("Unmapping (after align) 0x%08x (%d bytes)\n",
                        virt_addr, mapping_size);
    virt_save = (address_t)virt_addr;
    #endif

    /* Unmap all pages needed */
    while((address_t)virt_addr < end_map)
    {
        /* Get PGDIR entry */
        pgdir_entry = (((address_t)virt_addr) >> 22);
        /* Get PGTABLE entry */
        pgtable_entry = (((address_t)virt_addr) >> 12) & 0x03FF;

        /* If page table not present no need to unmap */
        if((current_pgdir[pgdir_entry] & PG_STRUCT_ATTR_PRESENT) !=
           PG_STRUCT_ATTR_PRESENT)
        {
            return OS_ERR_MEMORY_NOT_MAPPED;
        }

        /* Get the address */
        pgtable_mapped= NULL;
        page_table = (uint32_t*)
                     ((address_t)current_pgdir[pgdir_entry] & 0xFFFFF000);
        page_entry = &pgtable_mapped[pgtable_entry];

        if((*page_entry & PG_STRUCT_ATTR_PRESENT) !=
           PG_STRUCT_ATTR_PRESENT)
        {
            return OS_ERR_MEMORY_NOT_MAPPED;
        }

        phy_addr = (void*)((address_t)page_entry & 0xFFFFF000);

        /* Release the frame */
        kernel_paging_free_frames(phy_addr, 1);

        *page_entry = PG_STRUCT_ATTR_KERNEL_ACCESS |
                      PG_STRUCT_ATTR_READ_ONLY |
                      PG_STRUCT_ATTR_NOT_PRESENT;

        /* Check if the whole page table is empty */
        for(i = 0; i < 1024; ++i)
        {
            if((pgtable_mapped[i] & PG_STRUCT_ATTR_PRESENT) == PG_STRUCT_ATTR_PRESENT)
            {
                break;
            }
        }
        if(i == 1024)
        {
            /* Free the page table  and mark the pgdir as not present */
            kernel_paging_free_frames(page_table, 1);
            current_pgdir[pgdir_entry] = PG_STRUCT_ATTR_NOT_PRESENT |
                                         PG_STRUCT_ATTR_READ_ONLY |
                                         PG_STRUCT_ATTR_KERNEL_ACCESS;
        }

        kernel_serial_debug("Unmapped 0x%08x\n", virt_addr);

        virt_addr = (uint8_t*)virt_addr + KERNEL_PAGE_SIZE;
    }

    #if PAGING_KERNEL_DEBUG == 1
    /* Get PGDIR entry */
    pgdir_entry = (((address_t)virt_save) >> 22);
    /* Get PGTABLE entry */
    pgtable_entry = (((address_t)virt_save) >> 12) & 0x03FF;


    #endif

    invalidate_tlb();

    return OS_NO_ERR;
}

void* paging_get_phys_address(const void* virt_addr)
{
    address_t  phys_addr;
    uint32_t   page_id;
    uint32_t   offset;
    address_t  pgdir_entry;
    address_t  pgtable_entry;
    uint64_t*  current_pgdir;
    uint64_t*  current_pgtable;
    uint64_t*  page_entry;

    page_id = (address_t)virt_addr & 0xFFFFF000;
    offset  = (address_t)virt_addr & 0x00000FFF;

    current_pgdir = (uint64_t*)&kernel_curr_pml4;

    /* Get PGDIR entry */
    pgdir_entry = (((address_t)page_id) >> 22);
    /* Get PGTABLE entry */
    pgtable_entry = (((address_t)page_id) >> 12) & 0x03FF;

    if((current_pgdir[pgdir_entry] & PG_STRUCT_ATTR_PRESENT) !=
        PG_STRUCT_ATTR_PRESENT)
    {
        return NULL;
    }

    /* Get the address */
    current_pgtable = (uint64_t*)current_pgdir[pgdir_entry];
    page_entry = &current_pgtable[pgtable_entry];

    phys_addr = ((address_t)page_entry & 0xFFFFF000);

    return (void*)(offset | phys_addr);
}