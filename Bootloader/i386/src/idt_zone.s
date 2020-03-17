;-------------------------------------------------------------------------------
; @file idt_zone.s
;
; @author Alexy Torres Aurora Dugo
;
; @date 06/03/2020
;
; @version 2.0
;
; @brief IDT data stored in the boot stage
;
; @details IDT data stored in the boot stage
;
; @copyright Alexy Torres Aurora Dugo
;-------------------------------------------------------------------------------

[bits 32]

;-------------------------------------------------------------------------------
; DEFINE Section
;-------------------------------------------------------------------------------

CODE32 equ 0x0008

;-----------------------------------------------------------
; DATA Section
;-----------------------------------------------------------

align 8
idt_base_:

; Entry 0
	dw 0x1000    ; Low 16 Bits of the handler address
	dw CODE32    ; Kernel CS
	db 0x00      ; Zero
	db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
	dw 0x0000    ; High 16 Bits of the handler address

; Entries 1 - 31
times 248 db 0

; Entry 32
	dw 0x2000    ; Low 16 Bits of the handler address
	dw CODE32    ; Kernel CS
	db 0x00      ; Zero
	db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
	dw 0x0000    ; High 16 Bits of the handler address

; Entries 33 - 79
times 376 db 0

; Entry 80
	dw 0x3000    ; Low 16 Bits of the handler address
	dw CODE32    ; Kernel CS
	db 0x00      ; Zero
	db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
	dw 0x0000    ; High 16 Bits of the handler address

; Entries 81 - 199
times 952 db 0

; Entry 200
	dw 0x4000    ; Low 16 Bits of the handler address
	dw CODE32    ; Kernel CS
	db 0x00      ; Zero
	db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
	dw 0x0000    ; High 16 Bits of the handler address

; Pad rest of the entries
times 440 db 0

idt_ptr_:                          ; IDT pointer for 16bit access
	dw idt_ptr_ - idt_base_ - 1    ; IDT size
	dd idt_base_                   ; IDT base

dw 0xDEAD