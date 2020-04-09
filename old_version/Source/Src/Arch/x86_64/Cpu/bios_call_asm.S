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

global _bios_call ; ASM entry point

_bios_call: