;-------------------------------------------------------------------------------
;
; File: spinlock.S
;
; See: critical.h
;
; Author: Alexy Torres Aurora Dugo
;
; Date: 21/02/2019
;
; Version: 1.0
;
; Spinlock synchronization primitive implementation.
; Implemented to be as fair as possible, even on hyperthreading systems.
;-------------------------------------------------------------------------------
[bits 64]

global pause_spinlock

section .text:

;-----------------
; Pause Spinlock
;
; Param:
;     Input: ESP + 4: Address of the lock
;----------------
pause_spinlock:
    push rbp
    mov  rbp, rsp
    push rax
    mov  rax, [rsp + 12]

pause_spinlock_entry:
    lock bts dword [rax], 0
    jc   pause_spinlock_pause

    pop  rax
    mov  rsp, rbp
    pop  rbp
    ret

pause_spinlock_pause:
    pause
    test  dword [rax], 1
    jnz   pause_spinlock_pause
    jmp   pause_spinlock_entry