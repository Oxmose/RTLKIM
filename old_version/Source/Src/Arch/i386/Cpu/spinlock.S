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

global pause_spinlock

section .text

;-----------------
; Pause Spinlock
;
; Param:
;     Input: ESP + 4: Address of the lock
;----------------
pause_spinlock:
    push ebp
    mov  ebp, esp
    push eax
    mov  eax, [esp + 12]

pause_spinlock_entry:
    lock bts dword [eax], 0
    jc   pause_spinlock_pause

    pop eax
    mov  esp, ebp
    pop  ebp
    ret

pause_spinlock_pause:
    pause
    test  dword [eax], 1
    jnz   pause_spinlock_pause
    jmp   pause_spinlock_entry