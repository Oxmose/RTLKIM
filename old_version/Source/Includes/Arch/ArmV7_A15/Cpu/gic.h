
/***************************************************************************//**
 * @file gic.h
 *
 * @see gic.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 09/05/2019
 *
 * @version 1.0
 *
 * @brief GIC management functions
 *
 * @details GIC management functions.Used to set the GIC and configure the 
 * module.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __GIC_H_
#define __GIC_H_

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/
/** @brief GICD CTLR register offset. */
#define GICD_CTLR_REG 0x1000

/** @brief GICC CTLR register offset. */
#define GICC_CTLR_REG 0x2000

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/* None. */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Enables the GIC.
 *
 * @details Enables the GIC interrupts by enabling the GICC and GICD CTLR 
 * registers. 
 */
void gic_enable(void);

/**
 * @brief Disables the GIC.
 *
 * @details Disables the GIC interrupts by disabling the GICC and GICD CTLR 
 * registers. 
 */
void gic_disable(void);

/** 
 * @brief Returns the GIC interrupt enable status.
 * 
 * @details Returns the GIC interrupt enable status by reading the GICC and GICD 
 * CTLR registers.
 * 
 * @return The function returns 0 is interrupts are disabled, >0 otherwise.
 */
uint32_t gic_get_status(void);

#endif /* __GIC_H_ */
