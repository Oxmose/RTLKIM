ENTRY(loader)                /* the name of the entry label */
SECTIONS
{
    /* Kernel starts at 2M */
    . = 0x200000;
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
        _edata  =  .;
    }

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
    kernel_heap_start = _end;

    /* End of kernel memory heap */
    kernel_heap_end = 0x3000000;

    /**********************************************
     * SPECIFIC DATA STRUCTURES BASE ADDRESS
     *********************************************/

    /* Booted CPU count, must be accessible in real mode */
    init_cpu_count = 0x1500;

    /* GDT must also be in real mode accessible memory */
    cpu_gdt_size = 0x1600; /* 16 bits */
    cpu_gdt_base = 0x1602; /* 32 bits */
    cpu_gdt      = 0x1606; /* SIZE = 16384bits, ends at 0x2406 */

    /* VESA Structures, must be placed in lower memory */
    vbe_info_base      = 0x2500; /* SIZE = 4096bits, ends at 0x2700 */
    vbe_mode_info_base = 0x2800; /* SIZE = 2048bits, ends at 0x2900 */

    /* CPU structures placed in between 1M and 2M */
    cpu_idt_size = 0x100000; /* 16 bit */
    cpu_idt_base = 0x100002; /* 32 bits*/
    cpu_idt      = 0x100006; /* SIZE = 16384bits, ends at 0x100806 */

    /* Multiboot data pointer */
    multiboot_data_ptr = 0x10100C; /* 32bits */
    memory_map_size    = 0x101010; /* 32bits */
    memory_map_data    = 0x101014; /* SIZE = 16384bits, ends at 0x101814 */
}
