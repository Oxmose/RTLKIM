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

;-----------------------------------------
; EXTERN FUNCTIONS
;-----------------------------------------
extern cpu_setup_gdt
extern cpu_setup_idt
extern cpu_setup_tss
extern setup_kheap
extern graphic_clear_screen

extern serial_init

extern kernel_kickstart

extern __bss_start
extern kernel_heap_end

extern multiboot_data_ptr

extern cpu_info                 ; CPU INFO structure

global kernel_stack             ; Kernel first stack
global low_loader               ; Kernel entry point
global loader                   ; Kernelk high mem entry point
;-----------------------------
; KERNEL STACK SETTINGS
;-----------------------------
KERNEL_STACK_SIZE equ 16384     ; DO NOT FORGET TO MODIFY IN CONFIG.H

;-----------------------------
; PAGING SETTINGS
;-----------------------------
KERNEL_MEM_OFFSET    equ 0xE0000000 ; DO NOT FORGET TO MODIFY IN CONFIG.H
KERNEL_START_PAGE_ID equ (KERNEL_MEM_OFFSET >> 22)

;-----------------------------
; MULTIBOOT SETTINGS
;-----------------------------
MODULEALIGN    equ 1<<0
MEMINFO        equ 1<<1

FLAGS       equ MODULEALIGN | MEMINFO
MAGIC       equ 0x1BADB002
CHECKSUM    equ -(MAGIC + FLAGS)

section .multiboot
  align 4
  multiboot_header:
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

section .text:

  low_loader equ (loader - KERNEL_MEM_OFFSET)

  align 4
  loader:
    ; Make sure interrupts are disabled and clear flags
    push  dword 0
    popfd

    ; Blank all uninitialized memory
    mov  edi, (__bss_start - KERNEL_MEM_OFFSET)
    xor  eax, eax
    blank_mem:
        mov  [edi], eax
        add  edi, 4
        cmp  edi, (kernel_heap_end - KERNEL_MEM_OFFSET)
        jl  blank_mem

    ; Map the first 4M of the kernel 1:1
    mov ecx, (pgdir_boot - KERNEL_MEM_OFFSET)
    mov cr3, ecx

    ; Enable 4MB pages
    mov ecx, cr4
    or  ecx, 0x00000010
    mov cr4, ecx

    ; Enable paging
    mov ecx, cr0
    or  ecx, 0x80000000
    mov cr0, ecx

    ; Load high mem kernel entry point
    lea ecx, [high_loader]
    jmp ecx

  high_loader:
    ; Unmap the first 4M
    mov dword [pgdir_boot], 0
    invlpg [0]

    ; Save Multiboot info to Memory
    mov edi, multiboot_data_ptr
    mov [edi], ebx

    ; Init stack pointer
    mov  esp, kernel_stack + KERNEL_STACK_SIZE

    ; Init base pointer
    xor ebp, ebp

    ; Init CPU info structure
    mov  edi, cpu_info
    mov  esi, cpu_info ; Compute the
    add  esi, 4        ; end address of the structure
    xor  eax, eax
    clear_cpu_info:
        mov  [edi], eax
        add  edi, 4
        cmp  edi, esi
        jl   clear_cpu_info

    call get_cpuid_flag ; Get CPUID instruction availability

    ; Clear screen
    call graphic_clear_screen

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


  get_cpuid_flag:
    pushfd                                  ;Save EFLAGS
    pushfd                                  ;Store EFLAGS
    xor    dword [esp],0x00200000           ;Invert the ID bit in stored EFLAGS
    popfd                                   ;Load stored EFLAGS
    pushfd                                  ;Store EFLAGS again
    pop    eax                              ;eax = modified EFLAGS
    xor    eax,[esp]                        ;eax = whichever bits were changed
    popfd                                   ;Restore original EFLAGS
    and    eax,0x00200000                   ;eax = 0 CPUID is not available
    mov    edi, cpu_info
    or     [edi], eax                       ; Store information in the structure
    ret

;-----------------------
; THE STACK SPACE
;-----------------------
section .bss
  align 32
  kernel_stack:
    resb KERNEL_STACK_SIZE

section .data
  align 0x1000
  pgdir_boot:
    ; 4MB R/W Present.
    dd 0x00000083
    times (KERNEL_START_PAGE_ID - 1) dd 0         ; Pages before kernel space.
    ; This page directory entry defines a 4MB page containing the kernel.
    dd 0x00000083
    ; This page directory entry defines a 4MB page containing the kernel.
    dd 0x00400083
    ; This page directory entry defines a 4MB page containing the kernel.
    dd 0x00800083
    ; This page directory entry defines a 4MB page containing the kernel.
    dd 0x00C00083
    times (1024 - KERNEL_START_PAGE_ID - 5) dd 0  ; Pages after the kernel image.