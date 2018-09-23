/***************************************************************************//**
 * @file config.h
 *
 * @see loader.S
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 09/03/2018
 *
 * @version 2.0
 *
 * @brief Kernel's main configuration file.
 * 
 * @details Kernel configuration's definitions. This file stores the different 
 * settings used when compiling RTLK.
 * 
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __CONFIG_H_
#define __CONFIG_H_

/** 
 * @brief Kernel's main stack size in bytes.
 * 
 * @warning When modifying this value, do not forget to modify it in the 
 * loader.S file.
 */
#define KERNEL_STACK_SIZE 16384

/** @brief Maximal number of CPU to be supported by the kernel. */
#define MAX_CPU_COUNT     1

/** @brief Enables SMP support. */
#define ENABLE_SMP        0

/** @brief Enables VESA support for graphic drivers. */
#define ENABLE_VESA       0

/** @brief Enables ATA drivers support. */
#define ENABLE_ATA        0

/** @brief Enables GUI support. */
#define ENABLE_GUI        0

/** @brief Enables mouse driver support. */
#define ENABLE_MOUSE      0

/** @brief Defines which serial port is used for debug purposes. */
#define SERIAL_DEBUG_PORT COM1

/* Screen settings */

/** @brief When VESA drivers are enabled, defines the maximal supported height
 * resolution.
 */
#define MAX_SUPPORTED_HEIGHT 1080
/** @brief When VESA drivers are enabled, defines the maximal supported width
 * resolution.
 */
#define MAX_SUPPORTED_WIDTH  1100
/** @brief When VESA drivers are enabled, defines the maximal supported color
 * depth.
 */
#define MAX_SUPPORTED_BPP    32

/*******************************************************************************
 * DEBUG CONFIGURATION
 ******************************************************************************/
/** @brief Enables kernel debuging features. */
#define KERNEL_DEBUG 1

/** @brief Enables PIC driver debuging feature. */
#define PIC_KERNEL_DEBUG 1

/** @brief Enables Serial driver debuging feature. */
#define SERIAL_KERNEL_DEBUG 1

/** @brief Enables test mode features. */
#define TEST_MODE_ENABLED 0

#endif /* __CONFIG_H_ */