/***************************************************************************//**
 * @file cpu.h
 *
 * @see cpu.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 03/10/2017
 *
 * @version 1.0
 *
 * @brief ARMV7 CPU management functions
 *
 * @details ARMV7 CPU manipulation functions. Wraps inline assembly calls for ease
 * of development.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __CPU_H_
#define __CPU_H_

#include <Lib/stdint.h> /* Generic int types */
#include <Lib/stddef.h> /* OS_RETURN_E */
#include <Cpu/gic.h>    /* gic_enable gic_disable */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/


/** @brief Clears interupt bit which results in disabling interrupts. */
__inline__ static void cpu_clear_interrupt(void)
{
    gic_disable();
}

/** @brief Sets interrupt bit which results in enabling interupts. */
__inline__ static void cpu_set_interrupt(void)
{
    gic_enable();
}

/** @brief Halts the CPU for lower energy consuption. */
__inline__ static void cpu_hlt(void)
{
    /* Todo */
}

/*******************************************************************************
 * Memory mapped IOs, avoids compilers to reorganize memory access
 *
 * So instead of doing : *addr = value, do
 * mapped_io_write(addr, value)
 ******************************************************************************/

/**
 * @brief Memory mapped IO byte write access.
 *
 * @details Memory mapped IOs byte write access. Avoids compilers to reorganize
 * memory access, instead of doing : *addr = value, do
 * mapped_io_write(addr, value) to avoid instruction reorganization.
 *
 * @param[in] addr The address of the IO to write.
 * @param[in] value The value to write to the IO.
 */
__inline__ static void mapped_io_write_8(void* volatile addr,
                                         const uint8_t value)
{
    *(volatile uint8_t*)(addr) = value;
}

/**
 * @brief Memory mapped IO half word write access.
 *
 * @details Memory mapped IOs half word write access. Avoids compilers to
 * reorganize memory access, instead of doing : *addr = value, do
 * mapped_io_write(addr, value) to avoid instruction reorganization.
 *
 * @param[in] addr The address of the IO to write.
 * @param[in] value The value to write to the IO.
 */
__inline__ static void mapped_io_write_16(void* volatile addr,
                                          const uint16_t value)
{
    *(volatile uint16_t*)(addr) = value;
}

/**
 * @brief Memory mapped IO word write access.
 *
 * @details Memory mapped IOs word write access. Avoids compilers to reorganize
 * memory access, instead of doing : *addr = value, do
 * mapped_io_write(addr, value) to avoid instruction reorganization.
 *
 * @param[in] addr The address of the IO to write.
 * @param[in] value The value to write to the IO.
 */
__inline__ static void mapped_io_write_32(void* volatile addr,
                                          const uint32_t value)
{
    *(volatile uint32_t*)(addr) = value;
}

/**
 * @brief Memory mapped IO double word write access.
 *
 * @details Memory mapped IOs double word write access. Avoids compilers to
 * reorganize memory access, instead of doing : *addr = value, do
 * mapped_io_write(addr, value) to avoid instruction reorganization.
 *
 * @param[in] addr The address of the IO to write.
 * @param[in] value The value to write to the IO.
 */
__inline__ static void mapped_io_write_64(void* volatile addr,
                                          const uint64_t value)
{
    *(volatile uint64_t*)(addr) = value;
}

/**
 * @brief Memory mapped IO byte read access.
 *
 * @details Memory mapped IOs byte read access. Avoids compilers to reorganize
 * memory access, instead of doing : value = *addr, do
 * mapped_io_read(addr, value) to avoid instruction reorganization.
 *
 * @param[in] addr The address of the IO to read.
 */
__inline__ static uint8_t mapped_io_read_8(const volatile void* addr)
{
    return *(volatile uint8_t*)(addr);
}

/**
 * @brief Memory mapped IO half word read access.
 *
 * @details Memory mapped IOs half word read access. Avoids compilers to
 * reorganize memory access, instead of doing : value = *addr, do
 * mapped_io_read(addr, value) to avoid instruction reorganization.
 *
 * @param[in] addr The address of the IO to read.
 */
__inline__ static uint16_t mapped_io_read_16(const volatile void* addr)
{
    return *(volatile uint16_t*)(addr);
}

/**
 * @brief Memory mapped IO word read access.
 *
 * @details Memory mapped IOs word read access. Avoids compilers to reorganize
 * memory access, instead of doing : value = *addr, do
 * mapped_io_read(addr, value) to avoid instruction reorganization.
 *
 * @param[in] addr The address of the IO to read.
 */
__inline__ static uint32_t mapped_io_read_32(const volatile void* addr)
{
    return *(volatile uint32_t*)(addr);
}

/**
 * @brief Memory mapped IO double word read access.
 *
 * @details Memory mapped IOs double word read access. Avoids compilers to
 * reorganize memory access, instead of doing : value = *addr, do
 * mapped_io_read(addr, value) to avoid instruction reorganization.
 *
 * @param[in] addr The address of the IO to read.
 */
__inline__ static uint64_t mapped_io_read_64(const volatile void* addr)
{
    return *(volatile uint64_t*)(addr);
}
#endif /* __CPU_H_ */
