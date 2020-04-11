;-------------------------------------------------------------------------------
;
; File: ap_boot.S
;
; Author: Alexy Torres Aurora Dugo
;
; Date: 30/03/2018
;
; Version: 1.0
;
; Kernel entry point for AP cpus
;-------------------------------------------------------------------------------
[bits 64]
;-----------------------------------------
; EXTERN FUNCTIONS
;-----------------------------------------
extern init_cpu_count
extern init_cpu_count_phys
extern cpu_gdt_size
extern cpu_idt_size
extern ap_cpu_stacks
extern ap_cpu_stack_size
extern smp_ap_core_init

CODE64 equ 0x0028
DATA64 equ 0x0030
CODE32 equ 0x0008
DATA32 equ 0x0010
CODE16 equ 0x0018
DATA16 equ 0x0020

BOOT_STACK equ 0x6000

%define CODE_LOCATION 0x4000
%define OFFSET_ADDR(addr)  (((addr) - ap_boot_loader) + CODE_LOCATION)

global ap_boot_loader_init
global ap_boot_loader

ap_boot_loader_init:
        push rsi 
        push rdi 
        push rcx 

        mov rax, cr3 
        mov [ap_boot_pgdir_], rax

        ; RELOCATE THE CODE TO LOW MEMORY SPACE

        mov  rsi, ap_boot_loader           ; Source address
        mov  rdi, CODE_LOCATION            ; Destination address
        mov  rcx, (ap_boot_loader_end - ap_boot_loader) ; Size

        cld                                ; Clear direction flag
        rep  movsb                         ; Copy the code to low memory space
        
        pop rcx 
        pop rsi 
        pop rdi        
        ret

[bits 16]
ap_boot_loader:
    cli 

    ; Canonical jump 
    jmp 0:OFFSET_ADDR(canonical_start_)

canonical_start_:
    ; Set the GDT for the current CPU
    lgdt [OFFSET_ADDR(gdt16_ptr_)]

    ; Set PMode
	mov eax, cr0
	inc eax
	mov cr0, eax

	jmp dword CODE32:OFFSET_ADDR(ap_boot_loader_pm_)  ; Jump to PM mode

[bits 32]
ap_boot_loader_pm_:
    cli 

    mov esp, BOOT_STACK
    mov ebx, esp

    ; Set PM segments 
    mov ax, DATA32
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
	mov ss, ax

    ; Set CR3
	mov eax, [OFFSET_ADDR(ap_boot_pgdir_)]
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
	jmp CODE64:OFFSET_ADDR(ap_boot_switch_lm_entry_)

[bits 64]
ap_boot_switch_lm_entry_:
    cli 

    ; Get stack index
    mov rax, init_cpu_count
    mov rbx, [rax]

    ; Now set the cpu stack
    mov rcx, ap_cpu_stack_size
    mov rax, [rcx]
    mul rbx

    ; Init stack pointer
    mov rbx, ap_cpu_stacks
    add rax, rbx
    mov rsp, rax

    ; Init base pointer
    mov rbp, rsp

    ; Init CPU IDT
    mov rax, cpu_idt_size
    lidt [rax]
    ; Init CPU GDT
    mov rax, cpu_gdt_size
    lgdt [rax]

    ; Ini segments
    mov ax, DATA64
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Jump to C entry
    mov rax, smp_ap_core_init
    jmp rax

inf_loop:
    cli 
    hlt 
    jmp inf_loop


; Set stack in LM
;-----------------------------------------------------------
; DATA Section
;-----------------------------------------------------------
gdt16_:                                     ; GDT descriptor table
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

gdt16_ptr_:                                ; GDT pointer for 16bit access
	dw gdt16_ptr_ - gdt16_ - 1             ; GDT limit
	dd gdt16_                              ; GDT base address

ap_boot_pgdir_: 
    dd 0x00000000
    dd 0x00000000

 ap_boot_loader_end: