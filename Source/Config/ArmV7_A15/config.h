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
 * settings used when compiling UTK.
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

/**
 * @brief Kernel's high memory offset.
 *
 * @warning Must be 4MB aligned.
 */
#define KERNEL_MEM_OFFSET 0x80010000

/*******************************************************************************
 * Architecture settings
 ******************************************************************************/

/** @brief Use ARMV7 architecture. */
#define ARCH_ARMV7  2
/** @brief Use versatil A15 BSP. */
#define BSP_A15_VERSATILE 2

/** @brief UTK current architecture. */
#define UTK_ARCH ARCH_ARMV7
/** @brief UTK current BSP. */
#define UTK_BSP  BSP_A15_VERSATILE

/*******************************************************************************
 * Features settings
 ******************************************************************************/
/** @brief Maximal number of CPU to be supported by the kernel. */
#define MAX_CPU_COUNT  1

/** @brief Display with serial driver. */
#define DISPLAY_SERIAL 0

/*******************************************************************************
 * ARMV7 Arch Settings
 ******************************************************************************/

/** @brief Enables support for graphic drivers. */
#define DISPLAY_TYPE       DISPLAY_SERIAL

/*******************************************************************************
 * Global Arch Settings
 ******************************************************************************/
/** @brief Defines which serial port is used for debug purposes. */
#define SERIAL_DEBUG_PORT  COM1

/*******************************************************************************
 * Timers settings
 ******************************************************************************/
/** @brief Defines the current year (usefull for the RTC). */
#define CURRENT_YEAR 2019

/** @brief Defines the kernel's main timer frequency. This will set the maximal
 * scheduling frequency. */
#define KERNEL_MAIN_TIMER_FREQ 200
/** @brief Defines the kernel's auxiliary timer frequency. */
#define KERNEL_AUX_TIMER_FREQ  20
/** @brief Defines the kernel's rtc timer frequency. */
#define KERNEL_RTC_TIMER_FREQ 16

/*******************************************************************************
 * Threads settings
 ******************************************************************************/

/** @brief Defines the maximal length of a thread's name. */
#define THREAD_MAX_NAME_LENGTH   32
/** @brief Defines the thread's maximal stack size in bytes. */
#define THREAD_MAX_STACK_SIZE    0x00400000  /* 4 MB */
/** @brief Defines the thread's kernel stack size in bytes. */
#define THREAD_KERNEL_STACK_SIZE 0x400 /* 1KB */

/*******************************************************************************
 * Peripherals settings
 ******************************************************************************/

/*******************************************************************************
 * DEBUG CONFIGURATION
 ******************************************************************************/
/** @brief Enables kernel debuging features. */
#define KERNEL_DEBUG 1

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

/** @brief Enables kernel heap debuging feature. */
#define KHEAP_KERNEL_DEBUG 0

/** @brief Enables kernel queue debuging feature. */
#define QUEUE_KERNEL_DEBUG 0

/** @brief Enables kernel scheduler debuging feature. */
#define SCHED_KERNEL_DEBUG 0

/** @brief Enables kernel mutex debuging feature. */
#define MUTEX_KERNEL_DEBUG 0

/** @brief Enables kernel semaphore debuging feature. */
#define SEMAPHORE_KERNEL_DEBUG 0

/** @brief Enables kernel mailbox debuging feature. */
#define MAILBOX_KERNEL_DEBUG 0

/** @brief Enables kernel queue debuging feature. */
#define USERQUEUE_KERNEL_DEBUG 0

/** @brief Enables kernel paging debuging feature. */
#define PAGING_KERNEL_DEBUG 0

/** @brief Enables test mode features. */
#define TEST_MODE_ENABLED 0

#endif /* __CONFIG_H_ */
