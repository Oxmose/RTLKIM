ENTRY(low_loader_)                /* the name of the entry label */
SECTIONS
{
    /* Kernel starts at 1M in physical addresse space
     * Add the offset DO NOT FORGET TO CHANGE IT IN CONFIG.h AND LOADER.S
     * MUST BE 512 MB aligned
     */
    KERNEL_MEM_OFFSET = 0xE0000000;

    . = 0x100000;

    /* Put the multiboot header */
    .multiboot ALIGN(4K) : AT(ADDR(.multiboot))
    {
        *(.multiboot)
    }

    . += KERNEL_MEM_OFFSET;
    _start = .;

    /* Executable code */
    .text ALIGN(4K) : AT(ADDR(.text) - KERNEL_MEM_OFFSET)
    {
        *(.text)
    }

    /* Read-only data. */
    .rodata ALIGN(4K) : AT(ADDR(.rodata) - KERNEL_MEM_OFFSET)
    {
        *(.rodata)
    }

    /* Read-write data */
    .data ALIGN(4K) : AT(ADDR(.data) - KERNEL_MEM_OFFSET)
    {
        *(.data)
    }
    _edata = .;
    /* FROM HERE EVERYTHING IS ZEROED */

    /* Read-write data and stack */
    .bss ALIGN(4K) : AT(ADDR(.bss) - KERNEL_MEM_OFFSET)
    {
        __bss_start = .;
        *(COMMON)
        *(.bss)
        __bss_end = .;
    }

    _end = .;

    kernel_static_limit = 0x00500000 + KERNEL_MEM_OFFSET;

    /* SIZE = 4096bits, ends at kernel_static_limit + 0x1000 */
    kernel_current_pgdir = kernel_static_limit;

    /* SIZE = 4096bits, ends at kernel_current_pgdir + 0x1000 */
    kernel_current_pgtable = kernel_current_pgdir + 0x1000;

    /* Beginning of kernel memory heap */
    kernel_heap_start = 0x01000000 + KERNEL_MEM_OFFSET;

    /* End of kernel memory heap */
    kernel_heap_end = 0x05000000 + KERNEL_MEM_OFFSET;

    . = kernel_heap_end;

    _kernel_end = .;

    /* END OF ZEROING ZONE */

    /**********************************************
     * SPECIFIC DATA STRUCTURES BASE ADDRESS
     *********************************************/

    /* !!! WARNING 0x7C00 to 0x8000 Are reserved for BIOS calls !!! */

    /* Booted CPU count, must be accessible in real mode */
    init_cpu_count_phys = 0x1000;
    init_cpu_count      = init_cpu_count_phys + KERNEL_MEM_OFFSET;

    /* VESA Structures, must be placed in lower memory */
    vbe_info_base      = 0x2000 + KERNEL_MEM_OFFSET; /* SIZE = 4096bits, ends at 0x3000 */
    vbe_mode_info_base = 0x3000 + KERNEL_MEM_OFFSET; /* SIZE = 2048bits, ends at 0x3800 */

    /* Init AP relocated code */
    init_ap_code = 0x4000 + KERNEL_MEM_OFFSET; /* SIZE = 2048bits, ends at 0x4800 */

    /* GDT must also be in real mode accessible memory */
    cpu_gdt      = 0x8000 + KERNEL_MEM_OFFSET; /* SIZE = 16384bits, ends at 0xC000 */
    cpu_gdt_size = 0xC000 + KERNEL_MEM_OFFSET; /* 16 bits */
    cpu_gdt_base = 0xC002 + KERNEL_MEM_OFFSET; /* 64 bits */

    /* CPU IDT */
    cpu_idt      = 0x0D000 + KERNEL_MEM_OFFSET; /* SIZE = 32768bits, ends at 0x15000 */
    cpu_idt_size = 0x15000 + KERNEL_MEM_OFFSET; /* 16 bit */
    cpu_idt_base = 0x15002 + KERNEL_MEM_OFFSET; /* 64 bits*/

    /* Multiboot data pointer */
    memory_map_data    = 0x16000 + KERNEL_MEM_OFFSET; /* SIZE = 16384bits, ends at 0x1A000 */
    multiboot_data_ptr = 0x1A000 + KERNEL_MEM_OFFSET; /* 64bits */
    memory_map_size    = 0x1A008 + KERNEL_MEM_OFFSET; /* 32bits */
}