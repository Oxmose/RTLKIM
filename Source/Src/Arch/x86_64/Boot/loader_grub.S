;-------------------------------------------------------------------------------
;
; File: loader32.S
;
; Author: Alexy Torres Aurora Dugo
;
; Date: 21/03/2020
;
; Version: 1.0
;
; Kernel entry point and cpu initialization
;-------------------------------------------------------------------------------
[bits 32]

;-------------------------------------------------------------------------------
; DEFINE Section
;-------------------------------------------------------------------------------

CODE64 equ 0x0028
DATA64 equ 0x0030
CODE32 equ 0x0008
DATA32 equ 0x0010
CODE16 equ 0x0018
DATA16 equ 0x0020

KERNEL_MEM_OFFSET    equ 0xE0000000 ; DO NOT FORGET TO MODIFY IN CONFIG.H
KERNEL_PML4_ENTRY    equ (KERNEL_MEM_OFFSET >> 39)
KERNEL_PDPT_ENTRY    equ (KERNEL_MEM_OFFSET >> 30)
KERNEL_PDT_ENTRY     equ (KERNEL_MEM_OFFSET >> 21)

multiboot_data_ptr_low_ equ (multiboot_data_ptr - KERNEL_MEM_OFFSET)

;-----------------------------
; GLOBAL FUNCTIONS
;-----------------------------

global low_loader_
global kernel_stack

;-----------------------------
; EXTERN FUNCTIONS
;-----------------------------

;-----------------------------
; EXTERN DATA
;-----------------------------

extern init_cpu_count_phys
extern multiboot_data_ptr
extern x86_64_kernel_entry_

;-----------------------------
; KERNEL STACK SETTINGS
;-----------------------------
KERNEL_STACK_SIZE equ 16384     ; DO NOT FORGET TO MODIFY IN CONFIG.H

;-----------------------------
; MULTIBOOT SETTINGS
;-----------------------------
MODULEALIGN     equ 1<<0
MEMINFO         equ 1<<1

FLAGS           equ MODULEALIGN | MEMINFO
MAGIC           equ 0x1BADB002
CHECKSUM        equ -(MAGIC + FLAGS)

;-----------------------------
; MULTIBOOT SECTION
;-----------------------------
section .multiboot
align 4
multiboot_header:
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

;-----------------------------
; BOOT SECTION
;-----------------------------
low_loader_:
    ; Make sure interrupts are disabled and clear flags
    push  dword 0
    popfd

    ; Save Multiboot info to Memory
    mov eax, multiboot_data_ptr_low_
    mov [eax], ebx

    ; Set basic IDT
	lgdt [gdt_tmp_ptr_]

    ; Update segments 
    jmp dword CODE32:loader_set_segments_

loader_set_segments_:
    mov eax, DATA32
    mov ds, ax
    mov es, ax 
    mov fs, ax 
    mov gs, ax 
    mov ss, ax

    ; Update booted CPU count
    mov eax, 1
    mov [init_cpu_count_phys], eax

    ; Init stack pointer
    mov  esp, kernel_stack + KERNEL_STACK_SIZE

	; Set the PML4T
	mov ebx, paging_pdpt0             ; First entry to PDPT0
	or  ebx, 0x3                      ; Present and writable
	mov eax, paging_pml4t
	mov [eax], ebx 

	; Blank the rest of the PML4T
	xor ebx, ebx
x86_64_paging_init_blank_pml4t_:
	add eax, 8
	cmp eax, paging_pdpt0
	mov [eax], ebx
	jne x86_64_paging_init_blank_pml4t_

	; Set the PDPT0
	mov ebx, paging_pdt0              ; First entry to PDT0
	or  ebx, 0x3                      ; Present and writable
	mov eax, paging_pdpt0
	mov [eax], ebx 

	; Blank the rest of the PDPT0
	xor ebx, ebx
x86_64_paging_init_blank_pdpt0_:
	add eax, 8
	cmp eax, paging_pdt0
	mov [eax], ebx
	jne x86_64_paging_init_blank_pdpt0_
	
	; Set the first 2MB PDT0
	mov ebx, 0x83                     ; Present, writable, 2MB page
	mov eax, paging_pdt0

x86_64_paging_init_blank_pdt0_:
	mov [eax], ebx 
	add eax, 8
	add ebx, 0x200000
    xor ebx, ebx ; The rest of the entries will be not present
	cmp eax, (paging_pdt0 + 0x1000)
	jne x86_64_paging_init_blank_pdt0_

    ; Set the 512 MB for the kernel

    ; Set the PML4 for kernel
    mov eax, KERNEL_PML4_ENTRY
    and eax, 0x1FF
    shl eax, 3
    ; If it is the same as the loader, skip
    mov ebx, paging_pdpt0
    cmp eax, 0    
    je  x86_64_paging_init_pdpt_

    add eax, paging_pml4t
    mov ebx, paging_pdptk
    or  ebx, 0x3                ; Present and writable
    mov [eax], ebx

x86_64_paging_init_pdpt_:
    and ebx, 0xFFFFFF00
    ; Set the PDPT for kernel
    mov eax, KERNEL_PDPT_ENTRY
    and eax, 0x1FF
    shl eax, 3
    
    add eax, ebx
    mov ebx, paging_pdtk
    or  ebx, 0x3                ; Present and writable
    mov [eax], ebx

x86_64_paging_init_pdt_:
    ; Set the PDT entries for kernel
    and ebx, 0xFFFFFF00
    mov eax, KERNEL_PDT_ENTRY
    and eax, 0x1FF
    shl eax, 3
    add eax, ebx
    mov ecx, eax
    add ecx, 0x800
	mov ebx, 0x83                     ; Present, writable, 2MB page
	
x86_64_paging_init_blank_pdtk_:
	mov [eax], ebx 
	add eax, 8
	add ebx, 0x200000
	cmp eax, ecx
	jne x86_64_paging_init_blank_pdtk_

    ; Long mode switch
	; Set CR3
	mov eax, paging_pml4t
	mov cr3, eax

	; Enable PAE
	mov eax, cr4 
	or  eax, 0x20
	mov cr4, eax 

	; Switch to compatibility mode 
	mov ecx, 0xC0000080
	rdmsr
	or  eax, 0x00000100
	wrmsr

	; Enable paging 
	mov eax, cr0
	or  eax, 0x80000000
	mov cr0, eax 

	; Far jump to 64 bit mode
	jmp CODE64:x86_64_kernel_entry_

[bits 64]
low_loader_halt_lm_:
    hlt
    jmp low_loader_halt_lm_
    
;-----------------------------------------------------------
; DATA Section
;-----------------------------------------------------------

pml4_same:
    db 0

;-----------------------
; TMP GDT
;-----------------------
align 32
gdt_tmp_:                                     ; GDT descriptor table
	.null_:
		dd 0x00000000
		dd 0x00000000

	.code_32_:
		dw 0xFFFF
		dw 0x0000
		db 0x00
		db 0x9A
		db 0xCF
		db 0x00

	.data_32_:
		dw 0xFFFF
		dw 0x0000
		db 0x00
		db 0x92
		db 0xCF
		db 0x00

	.code_16_:
		dw 0xFFFF
		dw 0x0000
		db 0x00
		db 0x9A
		db 0x0F
		db 0x00

	.data_16_:
		dw 0xFFFF
		dw 0x0000
		db 0x00
		db 0x92
		db 0x0F
		db 0x00

	.code_64_:
		dw 0xFFFF
		dw 0x0000
		db 0x00
		db 0x98
		db 0x20
		db 0x00

	.data_64_:
		dw 0xFFFF
		dw 0x0000
		db 0x00
		db 0x90
		db 0x20
		db 0x00

gdt_tmp_ptr_:                                ; GDT pointer for 16bit access
	dw gdt_tmp_ptr_ - gdt_tmp_ - 1           ; GDT limit
	dd gdt_tmp_                              ; GDT base address

;-----------------------
; INITIAL PAGE DIR
;-----------------------
align 0x1000
paging_pml4t:
    times 0x1000 db 0x00
paging_pdpt0:
    times 0x1000 db 0x00
paging_pdptk:
    times 0x1000 db 0x00
paging_pdt0:
    times 0x1000 db 0x00
paging_pdtk:
    times 0x1000 db 0x00

section .bss
;-----------------------
; STACK SPACE
;-----------------------

align 32
kernel_stack:
    resb KERNEL_STACK_SIZE