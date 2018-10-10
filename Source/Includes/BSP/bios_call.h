/***************************************************************************//**
 * @file bios_call.h
 * 
 * @see bios_call.c, bios_call_asm.S
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

#ifndef __BIOS_CALL_H_
#define __BIOS_CALL_H_

#include <Lib/stdint.h> /* Generic int types */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/** @brief BIOS call CPU abstraction. Used to store the CPU registers value. */
struct bios_int_regs
{
    /** @brief CPU di register */
    volatile uint16_t di;
    /** @brief CPU si register */
    volatile uint16_t si;
    /** @brief CPU bp register */
    volatile uint16_t bp;
    /** @brief CPU sp register */
    volatile uint16_t sp;
    /** @brief CPU bx register */
    volatile uint16_t bx;
    /** @brief CPU dx register */
    volatile uint16_t dx;
    /** @brief CPU cx register */
    volatile uint16_t cx;
    /** @brief CPU ax register */
    volatile uint16_t ax;
    /** @brief CPU gs register */
    volatile uint16_t gs;
    /** @brief CPU fs register */
    volatile uint16_t fs;
    /** @brief CPU es register */
    volatile uint16_t es;
    /** @brief CPU ds register */
    volatile uint16_t ds;
    /** @brief CPU eflags register */
    volatile uint16_t eflags;
} __attribute__((__packed__));

/** 
 * @brief Defines bios_int_regs_t type as a shorcut for struct bios_int_regs.
 */
typedef struct bios_int_regs bios_int_regs_t;


/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Issue a bios interrupt.
 *
 * @details Switch the CPU to real mode and raise an interrupt. This interrupt
 * should be handled by the BIOS IVT.
 * 
 * @param[in] intnum The interrupt number to raise.
 * @param[in] regs The array containing the registers values for the call.
 */
void bios_call(const uint32_t intnum, bios_int_regs_t* regs);

#endif /* __BIOS_CALL_H_  */