#include <stdio.h>
int main()
{
	for(int i = 0; i < 256; ++i)
{
	printf("/**\n * @brief Assembly interrupt handler for line %d. \n * Saves the context and call the generic interrupt handler\n */\n", i);
	printf("extern void interrupt_handler_%d(void);\n", i);
}
	return 0;
}
