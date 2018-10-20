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

#include <Lib/stdint.h>    /* Generic int types */
#include <Sync/critical.h> /* ENTER_CRITICAL, EXIT_CRITICAL */
#include <Memory/paging.h> /* kernel_mmap */
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

void bios_call(uint32_t intnum, bios_int_regs_t* regs)
{
	uint32_t  word;
	OS_RETURN_E err;

	ENTER_CRITICAL(word);

	/* Map the RM core */
	err = kernel_mmap((void*)0x0000, (void*)0x0000, 0x1000 * 1024,
	                  PG_DIR_FLAG_PAGE_SIZE_4KB |
                      PG_DIR_FLAG_PAGE_SUPER_ACCESS |
                      PG_DIR_FLAG_PAGE_READ_WRITE,
                      1);
	if(err != OS_NO_ERR)
	{
		return;
	}

	_bios_call(intnum, regs);

	EXIT_CRITICAL(word);
}