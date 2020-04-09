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

;-----------------------------------------
; EXTERN FUNCTIONS
;-----------------------------------------
extern init_cpu_count
extern cpu_gdt_ptr
extern cpu_idt_ptr
extern ap_cpu_stacks
extern ap_cpu_stack_size
extern smp_ap_core_init
extern kernel_pgdir

%define CODE32 0x08
%define DATA32 0x10
%define CODE16 0x18
%define DATA16 0x20

%define CODE_LOCATION 0x4000
%define OFFSET_ADDR(addr)  (((addr) - ap_boot_loader) + CODE_LOCATION)

global ap_boot_loader_init
global ap_boot_loader

;-----------------------------
; PAGING SETTINGS
;-----------------------------
KERNEL_MEM_OFFSET    equ 0xE0000000 ; DO NOT FORGET TO MODIFY IN CONFIG.H

use32
section .text
    align 4
    ap_boot_loader_init:
        push esi 
        push edi 
        push ecx 

        ; RELOCATE THE CODE TO LOW MEMORY SPACE

        mov  esi, ap_boot_loader           ; Source address
        mov  edi, CODE_LOCATION            ; Destination address
        mov  ecx, (ap_boot_loader_end - ap_boot_loader) ; Size

        cld                                ; Clear direction flag
        rep  movsb                         ; Copy the code to low memory space
        
        pop ecx 
        pop esi 
        pop edi        
        ret

  ap_boot_loader:
    use16
    cli
    ; Set the GDT for the current CPU
    lgdt [OFFSET_ADDR(gdt16_ptr)]

    ; Set PM
    mov eax, cr0
    ; set PE (Protection Enable) bit in CR0 (Control Register 0)
    or eax, 0x1
    mov cr0, eax

    jmp  dword CODE32:OFFSET_ADDR(ap_pm_mode)  ; Jump to PM mode

    ap_pm_mode:
      use32

      mov ax, DATA32

      mov ds, eax
      mov es, eax
      mov fs, eax
      mov gs, eax
      mov ss, eax

      ; Set CR3
      mov eax, kernel_pgdir
      add eax, -KERNEL_MEM_OFFSET
      mov cr3, eax

      ; Enable PSE
      mov eax, cr4
      or eax, 0x10
      mov cr4, eax

      ; Enable paging and set correct CR0
      mov eax, 0x80010011
      mov cr0, eax

      ; Get stack index
      mov ebx, [init_cpu_count]
      mov ecx, init_cpu_count

      ; Now set the cpu stack
      mov eax, [ap_cpu_stack_size]
      mul ebx

      ; Init stack pointer
      mov ebx, ap_cpu_stacks
      add eax, ebx
      mov esp, eax

      ; Init base pointer
      mov ebp, esp

      ; Init CPU IDT
      lidt [cpu_idt_ptr]

      ; Init CPU GDT
      lgdt [cpu_gdt_ptr]

      mov eax, smp_ap_core_init
      call eax

      ; We should never return

  loop:
    cli
    hlt
    jmp loop                   ; loop forever

    gdt16:                                     ; GDT descriptor table
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

    gdt16_ptr:                                 ; GDT pointer for 16bit access
        dw gdt16_ptr - gdt16 - 1               ; GDT limit
        dd gdt16                               ; GDT base address

 ap_boot_loader_end: