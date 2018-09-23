/***************************************************************************//**
 * @file pic.c
 * 
 * @see pic.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 17/12/2017
 *
 * @version 1.0
 *
 * @brief PIC (programmable interrupt controler) driver.
 * 
 * @details   PIC (programmable interrupt controler) driver. Allows to remmap 
 * the PIC IRQ, set the IRQs mask and manage EoI for the X86 PIC.
 * 
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/


#include <Cpu/cpu.h>          /* outb */
#include <Lib/stdint.h>       /* Generic int types */
#include <Lib/stddef.h>       /* OS_RETURN_E, NULL */
#include <IO/kernel_output.h> /* kernel_success */

/* RTLK configuration file */
#include <config.h>

/* Header file */
#include <Drivers/pic.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

OS_RETURN_E init_pic(void)
{
    /* Initialize the master, remap IRQs */
    outb(PIC_ICW1_ICW4 | PIC_ICW1_INIT, PIC_MASTER_COMM_PORT);
    outb(PIC0_BASE_INTERRUPT_LINE, PIC_MASTER_DATA_PORT);
    outb(0x4,  PIC_MASTER_DATA_PORT);
    outb(PIC_ICW4_8086,  PIC_MASTER_DATA_PORT);

    /* Initialize the slave, remap IRQs */
    outb(PIC_ICW1_ICW4 | PIC_ICW1_INIT, PIC_SLAVE_COMM_PORT);
    outb(PIC1_BASE_INTERRUPT_LINE, PIC_SLAVE_DATA_PORT);
    outb(0x2,  PIC_SLAVE_DATA_PORT);
    outb(PIC_ICW4_8086,  PIC_SLAVE_DATA_PORT);

    /* Set EOI for both PICs. */
    outb(PIC_EOI, PIC_MASTER_COMM_PORT);
    outb(PIC_EOI, PIC_SLAVE_COMM_PORT);

    /* Disable all IRQs */
    outb(0xFF, PIC_MASTER_DATA_PORT);
    outb(0xFF, PIC_SLAVE_DATA_PORT);

    return OS_NO_ERR;
}

OS_RETURN_E set_IRQ_PIC_mask(const uint32_t irq_number, const uint8_t enabled)
{
    uint8_t  init_mask;

    if(irq_number > PIC_MAX_IRQ_LINE)
    {
        return OS_ERR_NO_SUCH_IRQ_LINE;
    }

    /* Manage master PIC */
    if(irq_number < 8)
    {
        /* Retrieve initial mask */
        init_mask = inb(PIC_MASTER_DATA_PORT);

        /* Set new mask value */
        if(!enabled)
        {
            init_mask |= 1 << irq_number;
        }
        else
        {
            init_mask &= ~(1 << irq_number);
        }

        /* Set new mask */
        outb(init_mask, PIC_MASTER_DATA_PORT);
    }

    /* Manage slave PIC. WARNING, cascading has to be enabled */
    if(irq_number > 7)
    {        
        /* Set new IRQ number */
        uint32_t cascading_number = irq_number - 8;

        /* Retrieve initial mask */
        init_mask = inb(PIC_SLAVE_DATA_PORT);

        /* Set new mask value */
        if(!enabled)
        {
            init_mask |= 1 << cascading_number;
        }
        else
        {
            init_mask &= ~(1 << cascading_number);
        }

        /* Set new mask */
        outb(init_mask, PIC_SLAVE_DATA_PORT);
    }

    return OS_NO_ERR;
}

OS_RETURN_E set_IRQ_PIC_EOI(const uint32_t irq_number)
{
    if(irq_number > PIC_MAX_IRQ_LINE)
    {
        return OS_ERR_NO_SUCH_IRQ_LINE;
    }

    /* End of interrupt signal */
    if(irq_number > 7)
    {
        outb(PIC_EOI, PIC_SLAVE_COMM_PORT);
    }
    outb(PIC_EOI, PIC_MASTER_COMM_PORT);

    return OS_NO_ERR;
}