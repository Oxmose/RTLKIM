/***************************************************************************//**
 * @file bios_call.c
 * 
 * @see bios_call.h, bios_call_asm.S
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 02/01/2018
 *
 * @version 1.0
 *
 * @brief BIOS call manager.
 * 
 * @details BIOS call manager, allowing the CPU in protected mode to switch back
 * to real mode and issue an interrupt handled by the BIOS IVT.
 * 
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <Lib/stdint.h>           /* Generic int types */
#include <Interrupt/interrupts.h> /* kernel_interrupt_restore, 
                                   * kernel_interrupt_disable */

/* RTLK configuration file */
#include <config.h>


/* Header file */
#include <BSP/bios_call.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Assemly function */
extern void _bios_call(uint8_t intnum, bios_int_regs_t* regs);

void bios_call(uint8_t intnum, bios_int_regs_t* regs)
{
	uint32_t int_status;

	int_status = kernel_interrupt_disable();

	_bios_call(intnum, regs);

	kernel_interrupt_restore(int_status);
}