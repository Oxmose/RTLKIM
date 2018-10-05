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

/*******************************************************************************
 * Features settings 
 ******************************************************************************/
/** @brief Maximal number of CPU to be supported by the kernel. */
#define MAX_CPU_COUNT      1

/** @brief Enables the use of an IO-APIC instead of the PIC if present if the 
 * system.
 */
#define ENABLE_IO_APIC     1

/** @brief If the system allows it, use the LAPIC timer as main timer source.
 */
#define ENABLE_LAPIC_TIMER 1

/** @brief Enables SMP support. IO-APIC must be enable and supported to enable 
 * SMP.
 */
#define ENABLE_SMP         0

/** @brief Enables VESA support for graphic drivers. */
#define ENABLE_VESA        1

/** @brief Enables ATA drivers support. */
#define ENABLE_ATA         0

/** @brief Enables GUI support. */
#define ENABLE_GUI         0

/** @brief Enables mouse driver support. */
#define ENABLE_MOUSE       0

/** @brief Defines which serial port is used for debug purposes. */
#define SERIAL_DEBUG_PORT  COM1

/*******************************************************************************
 * Screen settings 
 ******************************************************************************/
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
 * Timers settings 
 ******************************************************************************/
/** @brief Defines the current year (usefull for the RTC). */
#define CURRENT_YEAR 2018

/** @brief  Defines the kernel's main timer frequency. This will set the maximal 
 * scheduling frequency. */
#define KERNEL_MAIN_TIMER_FREQ 20
/** @brief  Defines the kernel's auxiliary timer frequency. */
#define KERNEL_AUX_TIMER_FREQ  20
/** @brief  Defines the kernel's rtc timer frequency. */
#define KERNEL_RTC_TIMER_FREQ 2

/*******************************************************************************
 * DEBUG CONFIGURATION
 ******************************************************************************/
/** @brief Enables kernel debuging features. */
#define KERNEL_DEBUG 1

/** @brief Enables PIC driver debuging feature. */
#define PIC_KERNEL_DEBUG 0

/** @brief Enables Serial driver debuging feature. */
#define SERIAL_KERNEL_DEBUG 0

/** @brief Enables Interrupt debuging feature. */
#define INTERRUPT_KERNEL_DEBUG 0

/** @brief Enables Exception debugging feature. */
#define EXCEPTION_KERNEL_DEBUG 0

/** @brief Enables PIT driver debuging feature. */
#define PIT_KERNEL_DEBUG 0

/** @brief Enables RTC driver debuging feature. */
#define RTC_KERNEL_DEBUG 0

/** @brief Enables timers debuging feature. */
#define TIME_KERNEL_DEBUG 0

/** @brief Enables VESA driver debuging feature. */
#define VESA_KERNEL_DEBUG 0

/** @brief Enables kernel heap debuging feature. */
#define KHEAP_KERNEL_DEBUG 0

/** @brief Enables kernel acpi debuging feature. */
#define ACPI_KERNEL_DEBUG 0

/** @biref Enables kernel io apic debuging feature. */
#define IOAPIC_KERNEL_DEBUG 0

/** @biref Enables kernel lapic debuging feature. */
#define LAPIC_KERNEL_DEBUG 0

/** @brief Enables test mode features. */
#define TEST_MODE_ENABLED 0

#endif /* __CONFIG_H_ */