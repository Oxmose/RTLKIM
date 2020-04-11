[bits 32]
dd 1141       ; Kernel size in sectors
dd 0x00100000 ; Kernel start address
dd 0x00100000 ; Kernel entry point
dd 6 ; Kernel name size
db "UTKv1", 0     ; Kernel name
times 510-($-$$) db 0xFF
dw 0xE621
