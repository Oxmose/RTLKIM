;-------------------------------------------------------------------------------
;
; File: kinit_x86_64.S
;
; Author: Alexy Torres Aurora Dugo
;
; Date: 14/12/2017
;
; Version: 1.0
;
; Kernel entry point and cpu initialization
;-------------------------------------------------------------------------------
;-------------------------------------------------------------------------------
; ARCH
;-------------------------------------------------------------------------------
[bits 64]

;-------------------------------------------------------------------------------
; DEFINES
;-------------------------------------------------------------------------------

; Kernel memory layout 
KERNEL_MEM_OFFSET    equ 0xE0000000 ; WARNING This value should be updated to 
                                    ; fit other configuration files
KERNEL_START_PAGE_ID equ (KERNEL_MEM_OFFSET >> 22)

CODE_LOCATION equ 0x4000
%define OFFSET_ADDR(addr)  (((addr) - __ap_loader_start) + CODE_LOCATION)

;-------------------------------------------------------------------------------
; MACRO DEFINE
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
; EXTERN DATA
;-------------------------------------------------------------------------------
extern cpu_stacks
extern cpu_stack_size

extern init_cpu_count
extern cpu_info

extern _kernel_bss_start
extern _kernel_bss_end

extern multiboot_data_ptr

;-------------------------------------------------------------------------------
; EXTERN FUNCTIONS
;-------------------------------------------------------------------------------

extern serial_init
extern cpu_setup_gdt
extern cpu_setup_idt
extern cpu_setup_tss
extern kernel_kickstart

extern __ap_loader_start

;-------------------------------------------------------------------------------
; EXPORTED FUNCTIONS
;-------------------------------------------------------------------------------
global __kinit_x86_64
global __cpu_smp_loader_init

;-------------------------------------------------------------------------------
; CODE
;-------------------------------------------------------------------------------

section .text
align 4
;-------------------------------------------------------------------------------
; Kernel entry point
__kinit_x86_64:
    ; Make sure interrupts are disabled and clear flags
    push  0
    popfq

    ; Save multiboot pointer
    mov rax, multiboot_ptr
    mov [rax], rbx

    ; Init stack
    mov rax, cpu_stacks
    mov rbx, cpu_stack_size
    mov rbx, [rbx]
    add rax, rbx
    mov rsp, rax 
    mov rbp, rsp

    ; Init BSS
    mov  rdi, _kernel_bss_start
    mov  rax, _kernel_bss_end
    xor  rsi, rsi
__bss_init:
    mov  [rdi], rsi
    add  rdi, 8
    cmp  rdi, rax
    jb   __bss_init

    ; Enable NXE bit 
    mov ecx, 0xC0000080
    rdmsr
    or  eax, 0x00000800
    wrmsr

    ; Update multiboot ptr 
    mov rax, multiboot_ptr
    mov rbx, [rax]
    mov rax, multiboot_data_ptr
    mov [rax], rbx

    ; Update the booted CPU count
    mov rbx, 1
    mov rax, init_cpu_count
    mov [rax], rbx

    ; Enable SSE
    fninit
    mov rax, cr0
    and rax, 0xFFFFFFFFFFFFFFFB
    or  rax, 0x0000000000000002
    mov cr0, rax
    mov rax, cr4 
    or  rax, 0x00000600
    mov cr4, rax

    ; Setup system pre-kickstart
    call __get_cpuid_flag
    call serial_init
    call cpu_setup_gdt
    call cpu_setup_idt
    call cpu_setup_tss

    ; Kickstart kernel
    call kernel_kickstart

__kinit__x86_64_end:
    ; Disable interrupt and loop forever
    cli 
    hlt
    jmp __kinit__x86_64_end

;-------------------------------------------------------------------------------
; CPUID probing function
__get_cpuid_flag:
    pushfq                                  ;Save EFLAGS
    pushfq                                  ;Store EFLAGS

    xor    dword [rsp], 0x00200000          ;Invert the ID bit in stored EFLAGS

    popfq                                   ;Load stored EFLAGS
    pushfq                                  ;Store EFLAGS again
    pop    rax                              ;eax = modified EFLAGS

    xor    rax, [rsp]                       ;eax = whichever bits were changed

    popfq                                   ;Restore original EFLAGS

    and    rax, 0x00200000                  ;eax = 0 CPUID is not available
    mov    rdi, cpu_info
    or     [rdi], rax                       ; Store information in the structure

    ret

;-----------------------------------------------------------
; DATA Section
;-----------------------------------------------------------

section .data 
align 4
multiboot_ptr:
dd 0x00000000
dd 0x00000000