#include <stdio.h>

int main()
{
    unsigned int i;

    printf("align 8\nidt_base_:\n\n");

    for(i = 0; i < 256; ++i)
    {
        printf("dw int_%d_   ; Low 16 Bits of the handler address\n", i);
	    printf("dw CODE32    ; Kernel CS\n");
	    printf("db 0x00      ; Zero\n");
	    printf("db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present\n");
	    printf("dw 0x0000    ; High 16 Bits of the handler address\n");

        printf("\n; ----\n");
    }

    printf("idt_ptr_:                          ; IDT pointer for 16bit access\n");
	printf("dw idt_ptr_ - idt_base_ - 1    ; IDT size\n");
	printf("dd idt_base_                   ; IDT base\n\n");

    for(i = 0; i < 256; ++i)
    {
        printf("int_%d_:\n\t", i);
        printf("add eax, %d\n\t", i);
        printf("hlt\n\t");
        printf("jmp int_%d_\n", i);
    }

    return 0;
}