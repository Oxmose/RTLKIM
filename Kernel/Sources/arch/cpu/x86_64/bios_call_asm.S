;-------------------------------------------------------------------------------
;
; File: bios_call_asm.S
;
; Author: Alexy Torres Aurora Dugo
;
; Date: 02/01/2018
;
; Version: 1.0
;
; Switch to real mode to do a bios call then switch back to protected mode.
;-------------------------------------------------------------------------------
[bits 64]

global __bios_call ; ASM entry point

struc regs16_t
    .di resw 1
    .si resw 1
    .bp resw 1
    .sp resw 1
    .bx resw 1
    .dx resw 1
    .cx resw 1
    .ax resw 1
    .gs resw 1
    .fs resw 1
    .es resw 1
    .ds resw 1
    .ef resw 1
endstruc

;-----------------------------------------------------------
; CODE REALOCATION
;-----------------------------------------------------------
%define CODE_LOCATION      0x7100 ; Leave space for stack
%define OFFSET_ADDR(addr)  (((addr) - low_mem) + CODE_LOCATION)

;-----------------------------------------------------------
; SEGMENT DESCRIPTOR
;-----------------------------------------------------------
%define CODE32 0x08
%define DATA32 0x10
%define CODE16 0x18
%define DATA16 0x20

;-----------------------------------------------------------
; 16 BITS STACK
;-----------------------------------------------------------
%define STACK_16 (CODE_LOCATION - regs16_t_size)


section .text
    __bios_call:
        ret