@-------------------------------------------------------------------------------
@
@ File: loader.S
@
@ Author: Alexy Torres Aurora Dugo
@
@ Date: 03/05/2019
@
@ Version: 1.0
@
@ Kernel entry point and cpu initialization
@-------------------------------------------------------------------------------

@-----------------------------------------
@ EXTERN FUNCTIONS
@-----------------------------------------
.extern kernel_stack_end
.extern kernel_kickstart
.extern _edata
.extern _end

.global loader @ Kernel entry point

@-----------------------------
@ PAGING SETTINGS
@-----------------------------


@-----------------------------
@ BOOTSTRAP CODE
@-----------------------------

@ Load the kernel
loader:
    @ Set kernel stack
    ldr sp, =kernel_stack_end

@ Blank all uninitialized memory
blank_mem: 
    mov r0, #0
    ldr r1, =(_edata)
    ldr r2, =(_end)

blank_mem_loop:        
    str r0, [r1]
    add r1, r1, #1
    cmp  r1, r2
    blt  blank_mem_loop

@ Kernel initialization
init:
    @ Clear screen
    bl graphic_clear_screen

    @ Init serial for debug
    bl serial_init

    @ Setup the kernel heap
    @bl setup_kheap

    @ Kickstart the kernel 
    bl kernel_kickstart

loop:
    b loop
