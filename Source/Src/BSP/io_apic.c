/***************************************************************************//**
 * @file io_apic.c
 * 
 * @see io_apic.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 25/12/2017
 *
 * @version 1.0
 *
 * @brief IO-APIC (IO advanced programmable interrupt controler) driver.
 * 
 * @details IO-APIC (IO advanced programmable interrupt controler) driver.
 * Allows to remmap the IO-APIC IRQ, set the IRQs mask and manage EoI for the 
 * X86 IO-APIC.
 * 
 * @warning This driver also use the LAPIC driver to function correctly.
 * 
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <Cpu/cpu.h>              /* mapped_io */
#include <Lib/stdint.h>           /* Generic int types */
#include <Lib/stddef.h>           /* OS_RETURN_E, NULL */
#include <IO/kernel_output.h>     /* kernel_success */
#include <Interrupt/interrupts.h> /* INT_IRQ_OFFSET */
#include <BSP/acpi.h>             /* acpi_get_io_apic_address */
#include <BSP/lapic.h>            /* lapic_set_int_eoi */

/* RTLK configuration file */
#include <config.h>

/* Header file */
#include <BSP/io_apic.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/** @brief The IO APIC base address */
static const uint8_t* io_apic_base_addr;

/** @brief IO-APIC IRQ redirection count. */
static uint32_t max_redirect_count;

/** @brief IO_PIC driver instance. */
interrupt_driver_t io_apic_driver = {
    .driver_set_irq_mask    = io_apic_set_irq_mask,
    .driver_set_irq_eoi     = io_apic_set_irq_eoi,
    .driver_handle_spurious = io_apic_handle_spurious_irq
};

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Writes to the IO APIC controller memory.
 *
 * @details Writes to the IO APIC controller memory.
 * 
 * @param[in] reg The register to write.
 * @param[in] val The value to write to the register.
 */
__inline__ static void io_apic_write(const uint8_t reg, const uint32_t val)
{
    mapped_io_write_32((uint32_t*)(io_apic_base_addr + IOREGSEL), reg);
    mapped_io_write_32((uint32_t*)(io_apic_base_addr + IOWIN), val);
}

/**
 * @brief Reads into the IO APIC controller memory.
 * 
 * @details Reads into the IO APIC controller memory.
 *
 * @param[in] reg The register to read.
 * 
 * @return The value contained in the register.
 */
__inline__ static uint32_t io_apic_read(const uint8_t reg)
{
    mapped_io_write_32((uint32_t*)(io_apic_base_addr + IOREGSEL), reg);
    return mapped_io_read_32((uint32_t*)(io_apic_base_addr + IOWIN));
}

OS_RETURN_E io_apic_init(void)
{
    #if IOAPIC_KERNEL_DEBUG == 1
    kernel_serial_debug("IOAPIC initialization\n");
    #endif

    uint32_t    i;
    uint32_t    read_count;
    OS_RETURN_E err;

    /* Check IO-APIC support */
    #if ENABLE_IO_APIC == 0
    return OS_ERR_NOT_SUPPORTED;
    #endif 
    if(acpi_get_io_apic_available() == 0 || acpi_get_lapic_available() == 0)
    {
        return OS_ERR_NOT_SUPPORTED;
    }

    /* Get IO APIC base address */
    io_apic_base_addr = acpi_get_io_apic_address(0);

    /* Maximum entry count */
    read_count = io_apic_read(IOAPICVER);

    max_redirect_count = ((read_count >> 16) & 0xff) + 1;

    /* Disable all interrupts */
    for (i = 0; i < max_redirect_count; ++i)
    {
        err = io_apic_set_irq_mask(i, 0);
        if(err != OS_NO_ERR)
        {
            return err;
        }
    }

    return OS_NO_ERR;
}

OS_RETURN_E io_apic_set_irq_mask(const uint32_t irq_number, 
                                 const uint32_t enabled)
{
    uint32_t entry_lo   = 0;
    uint32_t entry_hi   = 0;
    uint32_t actual_irq = 0;

    if(irq_number >= max_redirect_count || irq_number > IO_APIC_MAX_IRQ_LINE)
    {
        return OS_ERR_NO_SUCH_IRQ_LINE;
    }

    /* Set the interrupt line */
    entry_lo |= irq_number + INT_IRQ_OFFSET;

    /* Set enable mask */
    entry_lo |= (~enabled & 0x1) << 16;

    /* Get the remapped value */
    actual_irq = acpi_get_remmaped_irq(irq_number);

    io_apic_write(IOREDTBL + actual_irq * 2, entry_lo);
    io_apic_write(IOREDTBL + actual_irq * 2 + 1, entry_hi);

    #if IOAPIC_KERNEL_DEBUG == 1
    kernel_serial_debug("IOAPIC mask IRQ %d (%d): %d\n",
                        irq_number, actual_irq, enabled);
    #endif

    return OS_NO_ERR;
}

OS_RETURN_E io_apic_set_irq_eoi(const uint32_t irq_number)
{
    #if IOAPIC_KERNEL_DEBUG == 1
    kernel_serial_debug("IOAPIC set IRQ EOI %d\n",
                        irq_number);
    #endif

    return lapic_set_int_eoi(irq_number);
}

INTERRUPT_TYPE_E io_apic_handle_spurious_irq(const uint32_t irq_number)
{
    #if IOAPIC_KERNEL_DEBUG == 1
    kernel_serial_debug("IOAPIC spurious IRQ %d\n",
                        irq_number);
    #endif
    
    lapic_set_int_eoi(irq_number);

    return INTERRUPT_TYPE_REGULAR;
}