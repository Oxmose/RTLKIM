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
#include <Memory/paging.h>       /* page fault handler */
#include <Boot/multiboot.h>      /* MULTIBOOT_MEMORY_AVAILABLE */
#include <Core/scheduler.h>      /* sched_get_thread_pgdir */
#include <Memory/arch_paging.h>  /* Paging information */
#include <Interrupt/exceptions.h> /* Exception management */
#include <Cpu/panic.h>            /* Kernel panic */

/* UTK configuration file */
#include <config.h>

/* Header file */
#include <Memory/paging.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

#define KERNEL_MIN_PGTABLE_SIZE  128
#define KERNEL_DYN_PGTABLE_ENTRY (KERNEL_MIN_PGTABLE_SIZE - 1)

/* Memory map data */
extern uint32_t    memory_map_size;
extern mem_range_t memory_map_data[];


uint32_t kernel_pgdir[1024] __attribute__((aligned(4096)));
uint32_t kernel_dyn_pgtable[1024] __attribute__((aligned(4096)));
static uint32_t min_pgtable[KERNEL_MIN_PGTABLE_SIZE][1024]
                                                __attribute__((aligned(4096)));

static uint32_t init = 0;
static uint32_t enabled = 0;

/** @brief Kernel code start (virtual). */
extern uint8_t _kernel_code_start;

/** @brief Kernel code end (virtual). */
extern uint8_t _kernel_code_end;

/** @brief Kernel read only data start (virtual). */
extern uint8_t _kernel_rodata_start;

/** @brief Kernel read only data end (virtual). */
extern uint8_t _kernel_rodata_end;

/** @brief Kernel data start (virtual). */
extern uint8_t _kernel_data_start;

/** @brief Kernel data end (virtual). */
extern uint8_t _kernel_data_end;

/** @brief Kernel bss start (virtual). */
extern uint8_t _kernel_bss_start;

/** @brief Kernel bss end (virtual). */
extern uint8_t _kernel_bss_end;

/** @brief Kernel structures start (virtual). */
extern uint8_t _kernel_struct_start;

/** @brief Kernel structures end (virtual). */
extern uint8_t _kernel_struct_end;

/** @brief Kernel static memory end (virtual). */
extern uint8_t _kernel_static_limit;

/** @brief Kernel heap start (virtual). */
extern uint8_t _kernel_heap_start;

/** @brief Kernel heap end (virtual). */
extern uint8_t _kernel_heap_end;

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

static void* map_pgtable(void* pgtable_addr)
{
    address_t  pgtable_entry;
    address_t  min_pgtable_entry;

    /* Get address entries */
    pgtable_entry = (((address_t)&kernel_dyn_pgtable) >> PG_TABLE_OFFSET) & 0x3FF;
    min_pgtable_entry = (((address_t)&kernel_dyn_pgtable - KERNEL_MEM_OFFSET) >> 
                            PG_DIR_OFFSET) & 0x3FF;

    /* Create the page table entry */
    min_pgtable[min_pgtable_entry][pgtable_entry] = 
        (address_t)pgtable_addr |
        PAGE_FLAG_SUPER_ACCESS |
        PAGE_FLAG_READ_WRITE |
        PAGE_FLAG_PRESENT;

    #if PAGING_KERNEL_DEBUG == 1
    kernel_serial_debug("Mapped dyn pgtable at 0x%p -> 0x%p\n", kernel_dyn_pgtable, 
                        pgtable_addr);
    #endif

    invalidate_tlb();

    return ((void*)&kernel_dyn_pgtable);
}

static void map_kernel_section(const void* start_addr, uint32_t size,
                               const uint8_t read_only)
{
    uint32_t pg_dir_entry;
    uint32_t pg_table_entry;
    uint32_t min_pgtable_entry;

    int64_t  to_map;
    address_t start_addr_align;


    /* Align start addr */
    start_addr_align = (address_t)start_addr & PG_ENTRY_MASK;

    /* Map pages */
    to_map = (address_t)start_addr - start_addr_align + size;
    while(to_map > 0)
    {
        /* Get entry indexes */
        pg_dir_entry      = (address_t)start_addr_align >> PG_DIR_OFFSET;
        pg_table_entry    = ((address_t)start_addr_align >> PG_TABLE_OFFSET) & 0x3FF;
        min_pgtable_entry = (((address_t)start_addr_align - KERNEL_MEM_OFFSET) >> 
                            PG_DIR_OFFSET) & 0x3FF;
        /* Create the page table */
        min_pgtable[min_pgtable_entry][pg_table_entry] = 
            (start_addr_align - KERNEL_MEM_OFFSET) |
            PAGE_FLAG_SUPER_ACCESS |
            (read_only ? PAGE_FLAG_READ_ONLY : PAGE_FLAG_READ_WRITE) |
            PAGE_FLAG_PRESENT;

        /* Set the page directory */
        kernel_pgdir[pg_dir_entry] = 
            ((address_t)&min_pgtable[min_pgtable_entry] - 
                KERNEL_MEM_OFFSET) |
            PG_DIR_FLAG_PAGE_SIZE_4KB |
            PG_DIR_FLAG_PAGE_SUPER_ACCESS |
            PG_DIR_FLAG_PAGE_READ_WRITE |
            PG_DIR_FLAG_PAGE_PRESENT;

        to_map -= KERNEL_PAGE_SIZE;
        start_addr_align += KERNEL_PAGE_SIZE;
    }

    #if PAGING_KERNEL_DEBUG == 1
    start_addr_align = (address_t)start_addr & PG_ENTRY_MASK;
    to_map = (address_t)start_addr - start_addr_align + size;
    kernel_serial_debug("Mapped kernel section at 0x%p -> 0x%p\n", start_addr_align, 
                        start_addr_align + to_map);
    #endif
}

 /**
 * @brief Handle a page fault exception.
 *
 * @details Handle a page fault exception raised by the cpu. The corresponding
 * registered handler will be called. If no handler is available, a panic is
 * raised.
 *
 * @param cpu_state The cpu registers structure.
 * @param int_id The exception number.
 * @param stack_state The stack state before the exception that contain cs, eip,
 * error code and the eflags register value.
 */
static void paging_fault_general_handler(cpu_state_t* cpu_state, address_t int_id,
                                         stack_state_t* stack_state)
{
    const mem_handler_t* cursor;
    address_t      fault_address;


    (void)cpu_state;

    __asm__ __volatile__ (
        "mov %%cr2, %%eax\n\t"
        "mov %%eax, %0\n\t"
    : "=m" (fault_address)
    : /* no input */
    : "%eax"
    );

    /* If the exception line is not right */
    if(int_id != PAGE_FAULT_LINE)
    {
        kernel_error("Divide by zero handler in wrong exception line.\n");
        panic(cpu_state, int_id, stack_state);
    }

    /* Search for handler */
    cursor = paging_get_handler_list();
    while(cursor)
    {
        if(cursor->start <= fault_address && cursor->end > fault_address)
        {
            break;
        }
        cursor = cursor->next;
    }

    /* Check handler availability */
    if(cursor == NULL)
    {
        panic(cpu_state, int_id, stack_state);
    }

    /* Call handler */
    cursor->handler(fault_address);    
}

static OS_RETURN_E kernel_mmap_internal(const void* virt_addr,
                                const void* phys_addr,
                                const uint32_t mapping_size,
                                const uint8_t read_only,
                                const uint8_t exec,
                                const uint8_t cache_enabled)
{
    uint32_t    pgdir_entry;
    uint32_t    pgtable_entry;
    uint32_t*   page_table;
    uint32_t*   page_entry;
    uint32_t    end_map;
    uint32_t    i;
    uint32_t*   new_frame = NULL;
    OS_RETURN_E err;

    (void)exec;

    if(init == 0)
    {
        return OS_ERR_PAGING_NOT_INIT;
    }

    /* Get end mapping addr */
    end_map = (address_t)virt_addr + mapping_size;

    #if PAGING_KERNEL_DEBUG == 1
    kernel_serial_debug("Mapping (before align) 0x%08x, to 0x%08x (%d bytes)\n",
                        virt_addr, phys_addr, mapping_size);
    #endif

    /* Align addr */
    virt_addr = (uint8_t*)((address_t)virt_addr & 0xFFFFF000);
    phys_addr = (uint8_t*)((address_t)phys_addr & 0xFFFFF000);

    #if PAGING_KERNEL_DEBUG == 1
    kernel_serial_debug("Mapping (after align) 0x%08x, to 0x%08x (%d bytes)\n",
                        virt_addr, phys_addr, mapping_size);
    #endif

    /* Map all pages needed */
    while((address_t)virt_addr < end_map)
    {
        /* Get PGDIR entry */
        pgdir_entry = (((address_t)virt_addr) >> 22);
        /* Get PGTABLE entry */
        pgtable_entry = (((address_t)virt_addr) >> 12) & 0x03FF;

        /* If page table not present create it */
        if((kernel_pgdir[pgdir_entry] & PG_DIR_FLAG_PAGE_PRESENT) !=
           PG_DIR_FLAG_PAGE_PRESENT)
        {
            new_frame = kernel_paging_alloc_frames(1, &err);

            if(new_frame == NULL)
            {
                break;
            }

            page_table = map_pgtable(new_frame);

            for(i = 0; i < 1024; ++i)
            {
                page_table[i] = PAGE_FLAG_SUPER_ACCESS |
                                PAGE_FLAG_READ_ONLY |
                                PAGE_FLAG_NOT_PRESENT;
            }

            kernel_pgdir[pgdir_entry] = (address_t)new_frame |
                                         PG_DIR_FLAG_PAGE_SIZE_4KB |
                                         PG_DIR_FLAG_PAGE_SUPER_ACCESS |
                                         PG_DIR_FLAG_PAGE_READ_WRITE |
                                         PG_DIR_FLAG_PAGE_PRESENT;
        }

        /* Map the address */
        page_table = (uint32_t*)
                     ((address_t)kernel_pgdir[pgdir_entry] & 0xFFFFF000);
        page_table = map_pgtable(page_table);
        page_entry = &page_table[pgtable_entry];

        /* Check if already mapped */
        if((*page_entry & PAGE_FLAG_PRESENT) == PAGE_FLAG_PRESENT)
        {
            #if PAGING_KERNEL_DEBUG == 1
            kernel_serial_debug("Mapping (after align) 0x%08x, to 0x%08x "
                                "(%d bytes) already mapped)\n",
                                virt_addr, virt_addr, mapping_size);
            #endif
            
            /* Unmap */
            if(new_frame != NULL)
            {
                kernel_paging_free_frames(new_frame, 1);
                kernel_pgdir[pgdir_entry] = 0;
            }
            return OS_ERR_MAPPING_ALREADY_EXISTS;
        }

        #if PAGING_KERNEL_DEBUG == 1
        kernel_serial_debug("Mapped (after align) 0x%p, to 0x%p\n",
                            virt_addr, phys_addr);
        #endif

        new_frame = NULL;

        *page_entry = (address_t)phys_addr |
            PAGE_FLAG_SUPER_ACCESS |
            (read_only ? PAGE_FLAG_READ_ONLY : PAGE_FLAG_READ_WRITE) |
            (cache_enabled ? PAGE_FLAG_CACHE_WB : PAGE_FLAG_CACHE_DISABLED) |
            PAGE_FLAG_PRESENT;


        virt_addr = (uint8_t*)virt_addr + KERNEL_PAGE_SIZE;
        phys_addr = (uint8_t*)phys_addr + KERNEL_PAGE_SIZE;
    }

    invalidate_tlb();

    return OS_NO_ERR;
}


OS_RETURN_E paging_init(void)
{
    uint32_t  i;
    void*     virt_addr;
    void*     new_frame;

    OS_RETURN_E err;

    /* Get the first range that is free */
    for(i = 0; i < memory_map_size; ++i)
    {
        if(memory_map_data[i].base != 0x00000000 &&
           memory_map_data[i].type == MULTIBOOT_MEMORY_AVAILABLE)
           {
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

    /* Init new page directory */
    for(i = 0; i < 1024; ++i)
    {
        kernel_pgdir[i] =
                      PG_DIR_FLAG_PAGE_SUPER_ACCESS |
                      PG_DIR_FLAG_PAGE_READ_ONLY |
                      PG_DIR_FLAG_PAGE_NOT_PRESENT;
    }

    /* Map kernel code */
    map_kernel_section(&_kernel_code_start, 
                       (address_t)&_kernel_code_end - 
                       (address_t)&_kernel_code_start,
                       1);

    /* Map kernel read only data */
    map_kernel_section(&_kernel_rodata_start, 
                       (address_t)&_kernel_rodata_end - 
                       (address_t)&_kernel_rodata_start,
                       1);

    /* Map kernel data */
    map_kernel_section(&_kernel_data_start, 
                       (address_t)&_kernel_data_end - 
                       (address_t)&_kernel_data_start,
                       0);

    /* Map kernel BSS */
    map_kernel_section(&_kernel_bss_start, 
                       (address_t)&_kernel_bss_end - 
                       (address_t)&_kernel_bss_start,
                       0);

    /* Map kernel config */
    map_kernel_section(&_kernel_struct_start, 
                       (address_t)&_kernel_struct_end - 
                       (address_t)&_kernel_struct_start,
                       1);

    /* Map kernel heap */
    map_kernel_section(&_kernel_heap_start, 
                       (address_t)&_kernel_heap_end - 
                       (address_t)&_kernel_heap_start,
                       0);

    /* Set CR3 register */
    __asm__ __volatile__("mov %%eax, %%cr3": :"a"((address_t)kernel_pgdir -
                                                  KERNEL_MEM_OFFSET));

    #if PAGING_KERNEL_DEBUG == 1
    kernel_serial_debug("CR3 Set to 0x%08x \n", kernel_pgdir);
    #endif

    enabled = 0;
    init = 1;

    err = paging_enable();
    if(err != OS_NO_ERR)
    {
        return err;
    }

     /* Map null pointer */
    new_frame = kernel_paging_alloc_frames(1, &err);
    if(new_frame == NULL)
    {
        return err;
    }

    virt_addr = map_pgtable(new_frame);

    ((uint32_t*)virt_addr)[0] = PAGE_FLAG_SUPER_ACCESS |
                                PAGE_FLAG_READ_WRITE |
                                PAGE_FLAG_NOT_PRESENT;

    kernel_pgdir[0] = (address_t)new_frame |
                      PG_DIR_FLAG_PAGE_SIZE_4KB |
                      PG_DIR_FLAG_PAGE_SUPER_ACCESS |
                      PG_DIR_FLAG_PAGE_READ_WRITE |
                      PG_DIR_FLAG_PAGE_PRESENT;

    /* Add page fault exception */
    err = kernel_exception_register_handler(PAGE_FAULT_LINE,
                                            paging_fault_general_handler);

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

OS_RETURN_E kernel_direct_mmap(const void* virt_addr,
                               const uint32_t mapping_size,
                               const uint8_t read_only,
                               const uint8_t exec)
{
    return kernel_mmap_internal(virt_addr, virt_addr, 
                                mapping_size, read_only, exec, 1);
}

OS_RETURN_E kernel_mmap_hw(const void* virt_addr,
                           const void* phys_addr,
                           const uint32_t mapping_size,
                           const uint8_t read_only,
                           const uint8_t exec)
{
    return kernel_mmap_internal(virt_addr, phys_addr, 
                                mapping_size, read_only, exec, 0);
}

OS_RETURN_E kernel_mmap(const void* virt_addr, 
                        const uint32_t mapping_size,
                        const uint8_t read_only,
                        const uint8_t exec)
{
    uint32_t    end_map;
    uint32_t    virt_save;
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

        err = kernel_mmap_internal(virt_addr, phys_addr, 0x1000, read_only, exec, 1);
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
    uint32_t    i;
    uint32_t*   pgtable_mapped;

    if(init == 0)
    {
        return OS_ERR_PAGING_NOT_INIT;
    }

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
    #endif

    /* Unmap all pages needed */
    while((address_t)virt_addr < end_map)
    {
        /* Get PGDIR entry */
        pgdir_entry = (((address_t)virt_addr) >> 22);
        /* Get PGTABLE entry */
        pgtable_entry = (((address_t)virt_addr) >> 12) & 0x03FF;

        /* If page table not present no need to unmap */
        if((kernel_pgdir[pgdir_entry] & PG_DIR_FLAG_PAGE_PRESENT) !=
           PG_DIR_FLAG_PAGE_PRESENT)
        {
            return OS_ERR_MEMORY_NOT_MAPPED;
        }

        /* Get the address */
        page_table = (uint32_t*)
                     ((address_t)kernel_pgdir[pgdir_entry] & 0xFFFFF000);
        pgtable_mapped = map_pgtable(page_table);
        page_entry = &pgtable_mapped[pgtable_entry];

        if((*page_entry & PAGE_FLAG_PRESENT) !=
           PAGE_FLAG_PRESENT)
        {
            return OS_ERR_MEMORY_NOT_MAPPED;
        }

        phy_addr = (void*)((address_t)page_entry & 0xFFFFF000);

        /* Release the frame */
        kernel_paging_free_frames(phy_addr, 1);

        *page_entry = PAGE_FLAG_SUPER_ACCESS |
                      PAGE_FLAG_READ_ONLY |
                      PAGE_FLAG_NOT_PRESENT;

        /* Check if the whole page table is empty */
        for(i = 0; i < 1024; ++i)
        {
            if((pgtable_mapped[i] & PAGE_FLAG_PRESENT) == PAGE_FLAG_PRESENT)
            {
                break;
            }
        }
        if(i == 1024)
        {
            /* Free the page table  and mark the pgdir as not present */
            kernel_paging_free_frames(page_table, 1);
            kernel_pgdir[pgdir_entry] = PG_DIR_FLAG_PAGE_NOT_PRESENT |
                                         PG_DIR_FLAG_PAGE_READ_ONLY |
                                         PG_DIR_FLAG_PAGE_SUPER_ACCESS;
        }

        kernel_serial_debug("Unmapped 0x%08x\n", virt_addr);

        virt_addr = (uint8_t*)virt_addr + KERNEL_PAGE_SIZE;
    }

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
    uint32_t*  current_pgtable;
    uint32_t*  page_entry;

    page_id = (address_t)virt_addr & 0xFFFFF000;
    offset  = (address_t)virt_addr & 0x00000FFF;

    /* Get PGDIR entry */
    pgdir_entry = (((address_t)page_id) >> 22);
    /* Get PGTABLE entry */
    pgtable_entry = (((address_t)page_id) >> 12) & 0x03FF;

    if((kernel_pgdir[pgdir_entry] & PG_DIR_FLAG_PAGE_PRESENT) !=
        PG_DIR_FLAG_PAGE_PRESENT)
    {
        return NULL;
    }

    /* Get the address */
    current_pgtable = (uint32_t*)kernel_pgdir[pgdir_entry];
    page_entry = &current_pgtable[pgtable_entry];

    phys_addr = ((address_t)page_entry & 0xFFFFF000);

    return (void*)(offset | phys_addr);
}