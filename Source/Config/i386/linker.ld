ENTRY(low_loader)                /* the name of the entry label */
SECTIONS
{
    /* Kernel starts at 1M in physical addresse space
     * Add the offset DO NOT FORGET TO CHANGE IT IN CONFIG.h AND LOADER.S
     */
    kernel_offset = 0xE0000000;

    . = kernel_offset;
    _start = .;
    . = kernel_offset + 0x00100000;


    /* Put the multiboot header */
    .multiboot ALIGN(4K) : AT(ADDR(.multiboot) - kernel_offset)
    {
        *(.multiboot)
    }

    /* Executable code */
    .text ALIGN(4K) : AT(ADDR(.text) - kernel_offset)
    {
        *(.text)
    }

    /* Read-only data. */
    .rodata ALIGN(4K) : AT(ADDR(.rodata) - kernel_offset)
    {
        *(.rodata)
    }

    /* Read-write data */
    .data ALIGN(4K) : AT(ADDR(.data) - kernel_offset)
    {
        *(.data)
    }
    _edata = .;
    /* FROM HERE EVERYTHING IS ZEROED */

    /* Read-write data and stack */
    .bss ALIGN(4K) : AT(ADDR(.bss) - kernel_offset)
    {
        __bss_start = .;
        *(COMMON)
        *(.bss)
        __bss_end = .;
    }

    _end = .;

    kernel_static_limit = 0x00500000 + kernel_offset;

    /* SIZE = 4096bits, ends at kernel_static_limit + 0x1000 */
    kernel_current_pgdir = kernel_static_limit;

    /* SIZE = 4096bits, ends at kernel_current_pgdir + 0x1000 */
    kernel_current_pgtable = kernel_current_pgdir + 0x1000;

    /* Beginning of kernel memory heap */
    kernel_heap_start = 0x00600000 + kernel_offset;

    /* End of kernel memory heap */
    kernel_heap_end = 0x01000000 + kernel_offset;

    . = kernel_heap_end;

    _kernel_end = .;

    /* END OF ZEROING ZONE */

    /**********************************************
     * SPECIFIC DATA STRUCTURES BASE ADDRESS
     *********************************************/

    /* !!! WARNING 0x7C00 to 0x8000 Are reserved for BIOS calls !!! */

    /* Booted CPU count, must be accessible in real mode */
    init_cpu_count_phys = 0x1000;
    init_cpu_count      = init_cpu_count_phys + kernel_offset;

    /* VESA Structures, must be placed in lower memory */
    vbe_info_base      = 0x2000 + kernel_offset; /* SIZE = 4096bits, ends at 0x3000 */
    vbe_mode_info_base = 0x3000 + kernel_offset; /* SIZE = 2048bits, ends at 0x3800 */

    /* Init AP relocated code */
    init_ap_code = 0x8000 + kernel_offset; /* SIZE = 2048bits, ends at 0x4800 */

    /* GDT must also be in real mode accessible memory */
    cpu_gdt      = 0x3000 + kernel_offset; /* SIZE = 16384bits, ends at 0xC000 */
    cpu_gdt_size = 0x7000 + kernel_offset; /* 16 bits */
    cpu_gdt_base = 0x7002 + kernel_offset; /* 32 bits */

    /* CPU IDT */
    cpu_idt      = 0x0D000 + kernel_offset; /* SIZE = 16384bits, ends at 0x11000 */
    cpu_idt_size = 0x11000 + kernel_offset; /* 16 bit */
    cpu_idt_base = 0x11002 + kernel_offset; /* 32 bits*/

    /* Multiboot data pointer */
    memory_map_data    = 0x12000 + kernel_offset; /* SIZE = 16384bits, ends at 0x16000 */
    multiboot_data_ptr = 0x16000 + kernel_offset; /* 32bits */
    memory_map_size    = 0x16002 + kernel_offset; /* 32bits */
}