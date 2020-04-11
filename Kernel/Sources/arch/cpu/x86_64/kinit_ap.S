;-------------------------------------------------------------------------------
;
; File: kinit_ap.S
;
; Author: Alexy Torres Aurora Dugo
;
; Date: 14/12/2017
;
; Version: 1.0
;
; Kernel AP entry point and cpu initialization
;-------------------------------------------------------------------------------
;-------------------------------------------------------------------------------
; ARCH
;-------------------------------------------------------------------------------
[bits 64]

;-------------------------------------------------------------------------------
; DEFINES
;-------------------------------------------------------------------------------

CODE_LOCATION equ 0x4000

; GDT segments 
CODE64 equ 0x0028
DATA64 equ 0x0030

;-------------------------------------------------------------------------------
; MACRO DEFINE
;-------------------------------------------------------------------------------

%define OFFSET_ADDR(addr)  (((addr) - __ap_loader_start) + CODE_LOCATION)

;-------------------------------------------------------------------------------
; EXTERN DATA
;-------------------------------------------------------------------------------

extern cpu_stack_size
extern cpu_stacks

extern cpu_idt_ptr
extern cpu_gdt_ptr

extern init_cpu_count

;-------------------------------------------------------------------------------
; EXTERN FUNCTIONS
;-------------------------------------------------------------------------------

extern cpu_ap_core_init

;-------------------------------------------------------------------------------
; EXPORTED FUNCTIONS
;-------------------------------------------------------------------------------

global __cpu_smp_loader_init

;-------------------------------------------------------------------------------
; CODE
;-------------------------------------------------------------------------------

section .text
align 4
;-------------------------------------------------------------------------------
; Kernel AP entry point
;-------------------------------------------------------------------------------

; Loader initialization
__cpu_smp_loader_init:
    push rsi 
    push rdi 
    push rcx 
    push rax

    

    ; Relocate boot code to low memory space
    mov  rsi, __ap_loader_start                     ; Source address
    mov  rdi, CODE_LOCATION                         ; Destination address
    mov  rcx, (__ap_loader_end - __ap_loader_start) ; Size

    cld                                ; Clear direction flag
    rep  movsb                         ; Copy the code to low memory space
    
    ; Save page directory
    mov rax, cr3
    mov [OFFSET_ADDR(_ap_boot_pgdir)], rax

    pop rax
    pop rcx 
    pop rdi 
    pop rsi        
    ret


[bits 16]
__ap_loader_start:
    cli 

    ; Canonical jump 
    jmp 0:OFFSET_ADDR(__canonical_start_)

__canonical_start_:
    ; Set the GDT for the current CPU
    lgdt [OFFSET_ADDR(_gdt_tmp_ptr)]

    ; Set PMode
    mov eax, cr0
    inc eax
    mov cr0, eax

    ; Jump to protected mode
    jmp dword (_gdt_tmp.code_32 - _gdt_tmp):OFFSET_ADDR(__ap_loader_pm) 

[bits 32]
__ap_loader_pm:
    cli 
    
    ; Set PM segments 
    mov ax, _gdt_tmp.data_32 - _gdt_tmp
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Set CR3
    mov eax, [OFFSET_ADDR(_ap_boot_pgdir)]
    mov cr3, eax

    ; Enable PAE
    mov eax, cr4 
    or  eax, 0x20
    mov cr4, eax 

    ; Switch to compatibility mode / NXE bit
    mov ecx, 0xC0000080
    rdmsr
    or  eax, 0x00000900
    wrmsr

    ; Enable paging 
    mov eax, cr0
    or  eax, 0x80010000
    mov cr0, eax 

    ; Far jump to 64 bit mode
    jmp CODE64:OFFSET_ADDR(__ap_loader_lm)

[bits 64]
__ap_loader_lm:
    cli 

	; Init data segments
    mov ax, DATA64
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ;mov ss, ax

	; Init CPU IDT
    mov rax, cpu_idt_ptr
    lidt [rax]

	 ; Init CPU GDT
    mov rax, cpu_gdt_ptr
    lgdt [rax]

    ; Get stack index
    mov rax, init_cpu_count
    mov rbx, [rax]
    inc rbx

    ; Now set the cpu stack
    mov rcx, cpu_stack_size
    mov rax, [rcx]
    mul rbx

    ; Init stack pointer
    mov rbx, cpu_stacks
    add rax, rbx
    mov rsp, rax

    ; Init base pointer
    mov rbp, rsp

    ; Jump to C entry
    mov rax, cpu_ap_core_init
    jmp rax

__ap_loader_end_loop:
    cli 
    hlt 
    jmp __ap_loader_end_loop

;-------------------------------------------------------------------------------
; DATA
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
; Boot temporary GDT
align 32
_gdt_tmp:                              
    .null:
        dd 0x00000000
        dd 0x00000000

    .code_32:
        dw 0xFFFF
        dw 0x0000
        db 0x00
        db 0x9A
        db 0xCF
        db 0x00

    .data_32:
        dw 0xFFFF
        dw 0x0000
        db 0x00
        db 0x92
        db 0xCF
        db 0x00

    .code_16:
        dw 0xFFFF
        dw 0x0000
        db 0x00
        db 0x9A
        db 0x0F
        db 0x00

    .data_16:
        dw 0xFFFF
        dw 0x0000
        db 0x00
        db 0x92
        db 0x0F
        db 0x00

    .code_64:
        dw 0xFFFF
        dw 0x0000
        db 0x00
        db 0x98
        db 0x20
        db 0x00

    .data_64:
        dw 0xFFFF
        dw 0x0000
        db 0x00
        db 0x90
        db 0x20
        db 0x00

_gdt_tmp_ptr:                      ; GDT pointer for 16bit access
    dw _gdt_tmp_ptr - _gdt_tmp - 1 ; GDT limit
    dd OFFSET_ADDR(_gdt_tmp)       ; GDT base address

;-------------------------------------------------------------------------------
; Boot page directory
_ap_boot_pgdir: 
    dd 0x00000000
    dd 0x00000000

__ap_loader_end: