;-------------------------------------------------------------------------------
;
; File: loader.S
;
; Author: Alexy Torres Aurora Dugo
;
; Date: 14/12/2017
;
; Version: 1.0
;
; Kernel entry point and cpu initialization
;-------------------------------------------------------------------------------

[bits 64]

;-----------------------------------------
; EXTERN FUNCTIONS
;-----------------------------------------
extern cpu_setup_gdt
extern cpu_setup_idt
extern cpu_setup_tss
extern setup_kheap
extern vga_clear_screen
extern serial_init
extern kernel_kickstart

extern init_cpu_count
extern kernel_stack
extern __bss_start
extern kernel_heap_end

global x86_64_kernel_entry_     ; Kernel entry point
global kernel_stack

;-----------------------------
; DEFINITIONS
;-----------------------------
KERNEL_STACK_SIZE equ 16384     ; DO NOT FORGET TO MODIFY IN CONFIG.H

section .text:
x86_64_kernel_entry_:

    ; Init stack pointer
    mov rsp, kernel_stack + KERNEL_STACK_SIZE
    mov rsp, rbp

    ; Update booted CPU count
    mov ebx, 1
    mov rax, init_cpu_count
    mov [rax], ebx

    ; Blank all uninitialized memory
    mov  rdi, (__bss_start)
    mov  rax, kernel_heap_end
    xor  rbx, rbx
    blank_mem:
        mov  [rdi], rbx
        add  rdi, 8
        cmp  rdi, rax
        jl  blank_mem

    ; Init stack pointer
    mov  rsp, kernel_stack + KERNEL_STACK_SIZE

    ; Init base pointer
    mov rbp, rsp

    ; Clear screen
    call vga_clear_screen

    ; Init serial for debug
    call serial_init

    ; Setup kernel GDT
    call cpu_setup_gdt

    ; Setup kernel IDT
    call cpu_setup_idt

    ; Setup kernel TSS
    call cpu_setup_tss

    ; Setup the kernel heap
    call setup_kheap

    ; Kickstart the kernel, we should never return from here.
    call kernel_kickstart

  ; Hang forever, this should never happen
  .loop:
    cli
    hlt
    jmp .loop                   ; loop forever


section .bss
;-----------------------
; STACK SPACE
;-----------------------

align 32
kernel_stack:
    resb KERNEL_STACK_SIZE