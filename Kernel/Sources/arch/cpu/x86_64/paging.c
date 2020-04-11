/*******************************************************************************
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

#include <lib/stddef.h>           /* Standard definitions */
#include <lib/stdint.h>           /* Generic int types */
#include <memory/meminfo.h>       /* Memory information */
#include <memory/memalloc.h>      /* Memory allocation */
#include <memory/paging.h>        /* Page fault handler */
#include <core/multiboot.h>       /* Multiboot memory information */
#include <arch_paging.h>          /* Paging information */
#include <interrupt/exceptions.h> /* Exception management */
#include <interrupt_settings.h>   /* Interrupt settings */
#include <core/panic.h>           /* Kernel panic */
#include <io/kernel_output.h>     /* Kernel output methods */
#include <sync/critical.h>        /* Critical sections */

/* UTK configuration file */
#include <config.h>

/* Header file */
#include <memory/paging.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/* Memory map data */
extern uint32_t    memory_map_size;
extern mem_range_t memory_map_data[];

/** @brief Kernel page directory. */
uint64_t kernel_pgdir[KERNEL_P4_SIZE] __attribute__((aligned(4096)));

/** @brief Kernel reserved page tables. */
static uint64_t min_pgtable[KERNEL_RESERVED_PAGING][KERNEL_P4_SIZE]
                                        __attribute__((aligned(4096)));

/** @brief Kernel reserved page tables head pointer */
static uint32_t min_pgtable_head = 0;

/** @brief Tells if paging is initialized. */
static uint32_t init    = 0;

/** @brief Tells if paging is enabled. */
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

#if MAX_CPU_COUNT > 1
/** @brief Critical section spinlock. */
static spinlock_t lock = SPINLOCK_INIT_VALUE;
#endif


/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

#define INVAL_PAGE(virt_addr)            \
{                                        \
    __asm__ __volatile__(                \
        "invlpg (%0)": :"r"(virt_addr) : "memory"     \
    );                                   \
}

#define INVAL_TLB(virt_addr)             \
{                                        \
    __asm__ __volatile__(                \
        "mov %%cr3, %%rax\n\tmov %%rax, %%cr3": : : "rax"     \
    );                                   \
}

#define CREATE_ENTRY(parent)                           \
{                                                      \
    void* new_frame = memalloc_alloc_kframes(1, &err); \
    if(err != OS_NO_ERR)                               \
    {                                                  \
        break;                                         \
    }                                                  \
                                                       \
    parent = (uintptr_t)new_frame |                    \
             PG_STRUCT_ATTR_4KB_PAGES |                \
             PG_STRUCT_ATTR_KERNEL_ACCESS |            \
             PG_STRUCT_ATTR_READ_WRITE |               \
             PG_STRUCT_ATTR_ENABLED_CACHE |            \
             PG_STRUCT_ATTR_WB_CACHE |                 \
             PG_STRUCT_ATTR_PRESENT;                   \
}

/** 
 * @brief Maps a kernel section to the memory.
 * 
 * @details Maps a kernel section to the memory. No frame are allocated as the 
 * memory should already be populated. This function is called before the actual
 * kernel page directory is put it place.
 * 
 * @param[in] start_addr The start address of the section.
 * @param[in] size The size of the section.
 * @param[in] read_only Set to 1 if the section is read only.
 * @param[in] exec Set to 1 if the section contains executable code.
 */
static void map_kernel_section(const void* start_addr, size_t size,
                               const uint8_t read_only, const uint8_t exec)
{
    uint16_t p4_entry;
    uint16_t p3_entry;
    uint16_t p2_entry;
    uint16_t p1_entry;

    uint64_t* p3_table;
    uint64_t* p2_table;
    uint64_t* p1_table;

    int64_t  to_map;
    uintptr_t start_addr_align;

    /* Align start addr */
    start_addr_align = (uintptr_t)start_addr & PAGE_ALIGN_MASK;

    /* Map pages */
    to_map = (uintptr_t)start_addr - start_addr_align + size;
    while(to_map > 0)
    {
        /* Get entry indexes */
        p4_entry = (uint16_t)((uintptr_t)start_addr_align >> P4_OFFSET) & 0x1FF;
        p3_entry = (uint16_t)((uintptr_t)start_addr_align >> P3_OFFSET) & 0x1FF;
        p2_entry = (uint16_t)((uintptr_t)start_addr_align >> P2_OFFSET) & 0x1FF;
        p1_entry = (uint16_t)((uintptr_t)start_addr_align >> P1_OFFSET) & 0x1FF;

        /* Check exising mapping in P4 */
        if(kernel_pgdir[p4_entry] & PG_STRUCT_ATTR_PRESENT)
        {
            p3_table = (uint64_t*)((kernel_pgdir[p4_entry] & PAGE_ALIGN_MASK) + 
                                   KERNEL_MEM_OFFSET);
        }
        else 
        {
            /* Allocated a new P3 entry */
            p3_table = (uint64_t*)&min_pgtable[min_pgtable_head++];
            kernel_pgdir[p4_entry] = ((uintptr_t)p3_table - KERNEL_MEM_OFFSET) |
                                      PG_STRUCT_ATTR_4KB_PAGES |
                                      PG_STRUCT_ATTR_KERNEL_ACCESS | 
                                      PG_STRUCT_ATTR_READ_WRITE |
                                      PG_STRUCT_ATTR_ENABLED_CACHE |
                                      PG_STRUCT_ATTR_WB_CACHE |
                                      PG_STRUCT_ATTR_PRESENT;
        }
        if(min_pgtable_head > KERNEL_RESERVED_PAGING)
        {
            kernel_error("Not enough paging reserved memory (needed at least %u)\n", 
                          min_pgtable_head);
            kernel_panic(OS_ERR_NO_MORE_FREE_MEM);
        }

        /* Check exising mapping in P3 */
        if(p3_table[p3_entry] & PG_STRUCT_ATTR_PRESENT)
        {
            p2_table = (uint64_t*)((p3_table[p3_entry] & PAGE_ALIGN_MASK) + 
                                   KERNEL_MEM_OFFSET);
        }
        else 
        {
            /* Allocated a new P3 entry */
            p2_table = (uint64_t*)&min_pgtable[min_pgtable_head++];
            p3_table[p3_entry] = ((uintptr_t)p2_table - KERNEL_MEM_OFFSET) |
                                 PG_STRUCT_ATTR_4KB_PAGES |
                                 PG_STRUCT_ATTR_KERNEL_ACCESS | 
                                 PG_STRUCT_ATTR_READ_WRITE |
                                 PG_STRUCT_ATTR_ENABLED_CACHE |
                                 PG_STRUCT_ATTR_WB_CACHE |
                                 PG_STRUCT_ATTR_PRESENT;
        }
        if(min_pgtable_head > KERNEL_RESERVED_PAGING)
        {
            kernel_error("Not enough paging reserved memory (needed at least %u)\n", 
                          min_pgtable_head);
            kernel_panic(OS_ERR_NO_MORE_FREE_MEM);
        }

        /* Check exising mapping in P2 */
        if(p2_table[p2_entry] & PG_STRUCT_ATTR_PRESENT)
        {
            p1_table = (uint64_t*)((p2_table[p2_entry] & PAGE_ALIGN_MASK) + 
                                   KERNEL_MEM_OFFSET);
        }
        else 
        {
            /* Allocated a new P3 entry */
            p1_table = (uint64_t*)&min_pgtable[min_pgtable_head++];
            p2_table[p2_entry] = ((uintptr_t)p1_table - KERNEL_MEM_OFFSET) |
                                 PG_STRUCT_ATTR_4KB_PAGES |
                                 PG_STRUCT_ATTR_KERNEL_ACCESS | 
                                 PG_STRUCT_ATTR_READ_WRITE |
                                 PG_STRUCT_ATTR_ENABLED_CACHE |
                                 PG_STRUCT_ATTR_WB_CACHE |
                                 PG_STRUCT_ATTR_PRESENT;
        }
        if(min_pgtable_head > KERNEL_RESERVED_PAGING)
        {
            kernel_error("Not enough paging reserved memory (needed at least %u)\n", 
                          min_pgtable_head);
            kernel_panic(OS_ERR_NO_MORE_FREE_MEM);
        }

        /* Create the page table */
        p1_table[p1_entry] = 
            (start_addr_align - KERNEL_MEM_OFFSET) |
            PG_STRUCT_ATTR_KERNEL_ACCESS |
            PG_STRUCT_ATTR_ENABLED_CACHE |
            PG_STRUCT_ATTR_WB_CACHE |
            PG_STRUCT_ATTR_4KB_PAGES | 
            (read_only ? PG_STRUCT_ATTR_READ_ONLY : PG_STRUCT_ATTR_READ_WRITE) |
            (exec ? 0 : PG_STRUCT_ATTR_NXE) |
            PG_STRUCT_ATTR_PRESENT;

        to_map -= KERNEL_PAGE_SIZE;
        start_addr_align += KERNEL_PAGE_SIZE;
    }

#if PAGING_KERNEL_DEBUG == 1
    start_addr_align = (uintptr_t)start_addr & PAGE_ALIGN_MASK;
    to_map = (uintptr_t)start_addr - start_addr_align + size;
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
static void paging_fault_general_handler(cpu_state_t* cpu_state, uintptr_t int_id,
                                         stack_state_t* stack_state)
{
    const mem_handler_t* cursor;
    uintptr_t            fault_address;

    /* If the exception line is not right */
    if(int_id != PAGE_FAULT_LINE)
    {
        kernel_error("Divide by zero handler in wrong exception line.\n");
        panic(cpu_state, int_id, stack_state);
    }

    __asm__ __volatile__ (
        "mov %%cr2, %%rax\n\t"
        "mov %%rax, %0\n\t"
    : "=m" (fault_address)
    : /* no input */
    : "%rax"
    );

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

/**
 * @brief Tells if a memory region is already mapped in the current page tables.
 * 
 * @details Tells if a memory region is already mapped in the current page 
 * tables. Returns 0 if the region is not mapped, 1 otherwise.
 * 
 * @return Returns 0 if the region is not mapped, 1 otherwise.
 */
uint32_t is_mapped(const uintptr_t start_addr, const size_t size)
{
    (void)start_addr;
    (void)size;

    return 0;
}

/** 
 * @brief Maps a virtual address to the corresponding physical address.
 * 
 * @details Maps a virtual address to the corresponding physical address.
 * The allocation should be done prior to using this function as all it 
 * does is mapping the addresses together.
 * 
 * @param[in] virt_addr The virtual start address of the region to map.
 * @param[in] phys_addr The physical start address of the region to map.
 * @param[in] mapping_size The size of the region to map.
 * @param[in] read_only Set to 1 if the region should be read only.
 * @param[in] exec Set to 1 if the region contains executable code.
 * @param[in] cache_enabled Set to 1 if the cache should be enabled for this 
 * region.
 * @param[in] hardware Set to 1 if this region is memory mapped hardware.
 */
static OS_RETURN_E kernel_mmap_internal(const void* virt_addr,
                                        const void* phys_addr,
                                        const size_t mapping_size,
                                        const uint8_t read_only,
                                        const uint8_t exec,
                                        const uint8_t cache_enabled,
                                        const uint8_t hardware)
{
    uintptr_t virt_align;
    uintptr_t phys_align;
    size_t    to_map;

    uint32_t int_state;

    uint16_t  p4_entry;
    uint16_t  p3_entry;
    uint16_t  p2_entry;
    uint16_t  p1_entry;
    uint64_t* p3_recur_addr;
    uint64_t* p2_recur_addr;
    uint64_t* p1_recur_addr;

    OS_RETURN_E err;

    /* Align addresses */
    virt_align = (uintptr_t)virt_addr & PAGE_ALIGN_MASK;
    phys_align = (uintptr_t)phys_addr & PAGE_ALIGN_MASK;

    /* Get mapping size */
    to_map = mapping_size + ((uintptr_t)virt_addr - virt_align);


#if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(int_state, &lock);
#else
    ENTER_CRITICAL(int_state);
#endif

    /* Check for existing mapping */
    if(is_mapped(virt_align, to_map))
    {
        return OS_ERR_MAPPING_ALREADY_EXISTS;
    }

    while(to_map)
    {
        /* Get entry indexes */
        p4_entry = (uint16_t)((uintptr_t)virt_align >> P4_OFFSET) & 0x1FF;
        p3_entry = (uint16_t)((uintptr_t)virt_align >> P3_OFFSET) & 0x1FF;
        p2_entry = (uint16_t)((uintptr_t)virt_align >> P2_OFFSET) & 0x1FF;
        p1_entry = (uint16_t)((uintptr_t)virt_align >> P1_OFFSET) & 0x1FF;
        p3_recur_addr = (uint64_t*)(P3_RECUR_BASE_ADDR  |
                                    p4_entry << P1_OFFSET);
        p2_recur_addr = (uint64_t*)(P2_RECUR_BASE_ADDR  | 
                                    p3_entry << P1_OFFSET |
                                    p4_entry << P2_OFFSET);
        p1_recur_addr = (uint64_t*)(P1_RECUR_BASE_ADDR  | 
                                    p2_entry << P1_OFFSET |
                                    p3_entry << P2_OFFSET |
                                    p4_entry << P3_OFFSET);
        /* Check presence in P4 */
        if((kernel_pgdir[p4_entry] & PG_STRUCT_ATTR_PRESENT) == 0)
        {
            CREATE_ENTRY(kernel_pgdir[p4_entry]);
        }

        /* Check presence in P3 */        
        if((p3_recur_addr[p3_entry] & PG_STRUCT_ATTR_PRESENT) == 0)
        {
            CREATE_ENTRY(p3_recur_addr[p3_entry]);
        }

        /* Check presence in P2 */
        if((p2_recur_addr[p2_entry] & PG_STRUCT_ATTR_PRESENT) == 0)
        {
            CREATE_ENTRY(p2_recur_addr[p2_entry]);
        }

        /* Check presence in P1 */
        if((p1_recur_addr[p1_entry] & PG_STRUCT_ATTR_PRESENT) == 0)
        {
            p1_recur_addr[p1_entry] = 
                phys_align | 
                PG_STRUCT_ATTR_4KB_PAGES |
                PG_STRUCT_ATTR_KERNEL_ACCESS | 
                (read_only ? PG_STRUCT_ATTR_READ_ONLY : PG_STRUCT_ATTR_READ_WRITE) |
                (cache_enabled ? PG_STRUCT_ATTR_WB_CACHE : PG_STRUCT_ATTR_DISABLED_CACHE) |
                (hardware ? PG_STRUCT_ATTR_HARDWARE : 0) |
                (exec ? 0 : PG_STRUCT_ATTR_NXE) |
                PG_STRUCT_ATTR_PRESENT;
        }
        

#if PAGING_KERNEL_DEBUG == 1
        kernel_serial_debug("Mapped page at 0x%p -> 0x%p\n", virt_align, 
                            phys_align);
#endif

        /* Update addresses and size */
        virt_align += KERNEL_PAGE_SIZE;
        phys_align += KERNEL_PAGE_SIZE;
        if(to_map >= KERNEL_PAGE_SIZE)
        {
            to_map -= KERNEL_PAGE_SIZE;
        }
        else 
        {
            to_map = 0;
        }
    }

#if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(int_state, &lock);
#else
    EXIT_CRITICAL(int_state);
#endif

    return OS_NO_ERR;
}


OS_RETURN_E paging_init(void)
{
    uint32_t    i;
    OS_RETURN_E err;

#if PAGING_KERNEL_DEBUG == 1
    kernel_serial_debug("Initializing paging\n");
#endif

    /* Initialize kernel page directory */
    for(i = 0; i < KERNEL_P4_SIZE; ++i)
    {
        kernel_pgdir[i] = 0;
    }

    /* Set recursive mapping */
    kernel_pgdir[KERNEL_P4_SIZE - 1] = 
            ((uintptr_t)(&kernel_pgdir) - KERNEL_MEM_OFFSET) |
            PG_STRUCT_ATTR_KERNEL_ACCESS |
            PG_STRUCT_ATTR_ENABLED_CACHE |
            PG_STRUCT_ATTR_WB_CACHE |
            PG_STRUCT_ATTR_4KB_PAGES | 
            PG_STRUCT_ATTR_READ_WRITE |
            PG_STRUCT_ATTR_NXE |
            PG_STRUCT_ATTR_PRESENT;

    /* Map kernel code */
    map_kernel_section(&_kernel_code_start, 
                       (uintptr_t)&_kernel_code_end - 
                       (uintptr_t)&_kernel_code_start,
                       1, 1);

    /* Map kernel read only data */
    map_kernel_section(&_kernel_rodata_start, 
                       (uintptr_t)&_kernel_rodata_end - 
                       (uintptr_t)&_kernel_rodata_start,
                       1, 0);

    /* Map kernel data */
    map_kernel_section(&_kernel_data_start, 
                       (uintptr_t)&_kernel_data_end - 
                       (uintptr_t)&_kernel_data_start,
                       0, 0);

    /* Map kernel BSS */
    map_kernel_section(&_kernel_bss_start, 
                       (uintptr_t)&_kernel_bss_end - 
                       (uintptr_t)&_kernel_bss_start,
                       0, 0);

    /* Map kernel config */
    map_kernel_section(&_kernel_struct_start, 
                       (uintptr_t)&_kernel_struct_end - 
                       (uintptr_t)&_kernel_struct_start,
                       1, 0);

    /* Map kernel heap */
    map_kernel_section(&_kernel_heap_start, 
                       (uintptr_t)&_kernel_heap_end - 
                       (uintptr_t)&_kernel_heap_start,
                       0, 0);                       
    

    /* Add page fault exception */
    err = kernel_exception_register_handler(PAGE_FAULT_LINE,
                                            paging_fault_general_handler);

        /* Set CR3 register */
    __asm__ __volatile__("mov %%rax, %%cr3": :"a"((uintptr_t)kernel_pgdir -
                                                  KERNEL_MEM_OFFSET));

    init = 1;

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
    __asm__ __volatile__("mov %%cr0, %%rax\n\t"
                         "or $0x80010000, %%eax\n\t"
                         "mov %%rax, %%cr0" : : : "rax");

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
    __asm__ __volatile__("mov %%cr0, %%rax\n\t"
                         "and $0x7FF7FFFF, %%rax\n\t"
                         "mov %%rax, %%cr0" : : : "rax");
#if PAGING_KERNEL_DEBUG == 1
    kernel_serial_debug("Paging disabled\n");
#endif

    enabled = 0;

    return OS_NO_ERR;
}

OS_RETURN_E kernel_mmap_hw(const void* virt_addr,
                           const void* phys_addr,
                           const size_t mapping_size,
                           const uint8_t read_only,
                           const uint8_t exec)
{
#if PAGING_KERNEL_DEBUG == 1
    kernel_serial_debug("Request HW mappping at 0x%p -> 0x%p (%uB)\n", 
                        virt_addr, 
                        phys_addr,
                        mapping_size);
#endif
    return kernel_mmap_internal(virt_addr, phys_addr, 
                                mapping_size, read_only, exec, 0, 1);
}

OS_RETURN_E kernel_mmap(const void* virt_addr, 
                        const size_t mapping_size,
                        const uint8_t read_only,
                        const uint8_t exec)
{
    void*       frames;
    uintptr_t   end_map;
    uintptr_t   start_map;
    size_t      page_count;
    OS_RETURN_E err;

    /* Compute physical memory size */
    end_map   = (uintptr_t)virt_addr + mapping_size;
    start_map = (uintptr_t)virt_addr & PAGE_ALIGN_MASK;

    if(end_map % KERNEL_PAGE_SIZE)
    {
        end_map &= PAGE_ALIGN_MASK;
        end_map += KERNEL_PAGE_SIZE;
    }
    page_count = (end_map - start_map) / KERNEL_PAGE_SIZE;

    /* Get a physical frame block */
    frames = memalloc_alloc_kframes(page_count, &err);
    if(err != OS_NO_ERR)
    {
        return err;
    }

#if PAGING_KERNEL_DEBUG == 1
    kernel_serial_debug("Request regular mappping at 0x%p -> 0x%p (%uB)\n",
                        virt_addr, 
                        frames,
                        mapping_size);
#endif

    err = kernel_mmap_internal(virt_addr, frames, 
                               mapping_size, read_only, exec, 1, 0);
    if(err != OS_NO_ERR)
    {
        /* Free allocated frames */
        memalloc_free_kframes(frames, page_count);
    }

    return err;
}

OS_RETURN_E kernel_munmap(const void* virt_addr, const size_t mapping_size)
{
    (void)virt_addr;
    (void)mapping_size;

    return OS_NO_ERR;
}