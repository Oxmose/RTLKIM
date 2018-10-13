ENTRY(loader)                /* the name of the entry label */
SECTIONS
{
    /* Kernel starts at 1M */
    . = 0x00100000;
    _start = .;

    /* Put the multiboot header */
    .multiboot ALIGN(4K) :
    {
        *(.multiboot)
    }

    /* Executable code */
    .text ALIGN(4K) :
    {
        *(.text)
    }

    /* Read-only data. */
    .rodata ALIGN(4K) :
    {
        *(.rodata)
    }

    /* Read-write data */
    .data ALIGN(4K) :
    {
        *(.data)
    }
    _edata = .;
    /* FROM HERE EVERYTHING IS ZEROED */

    /* Read-write data and stack */
    .bss ALIGN(4K) :
    {
        __bss_start = .;
        *(COMMON)
        *(.bss)
        __bss_end = .;
    }

    _end = .;

    /* Beginning of kernel memory heap */
    kernel_heap_start = 0x00300000;

    /* End of kernel memory heap */
    kernel_heap_end = 0x00D00000;

    /* END OF ZEROING ZONE */

    /**********************************************
     * SPECIFIC DATA STRUCTURES BASE ADDRESS
     *********************************************/

    /* !!! WARNING 0x7C00 to 0x8000 Are reserved for BIOS calls !!! */

    /* Booted CPU count, must be accessible in real mode */
    init_cpu_count = 0x500;

    /* VESA Structures, must be placed in lower memory */
    vbe_info_base      = 0x0600; /* SIZE = 4096bits, ends at 0x1600 */
    vbe_mode_info_base = 0x1600; /* SIZE = 2048bits, ends at 0x1E00 */

    /* !!! WARNING 0x7C00 to 0x8000 Are reserved for BIOS calls !!! */

    /* GDT must also be in real mode accessible memory */
    cpu_gdt      = 0x8000; /* SIZE = 16384bits, ends at 0xC000 */
    cpu_gdt_size = 0xC000; /* 16 bits */
    cpu_gdt_base = 0xC002; /* 32 bits */

    /* CPU IDT */
    cpu_idt      = 0x0D000; /* SIZE = 16384bits, ends at 0x11000 */
    cpu_idt_size = 0x11000; /* 16 bit */
    cpu_idt_base = 0x11002; /* 32 bits*/

    /* Multiboot data pointer */
    memory_map_data    = 0x12000; /* SIZE = 16384bits, ends at 0x16000 */
    multiboot_data_ptr = 0x16000; /* 32bits */
    memory_map_size    = 0x16002; /* 32bits */
}