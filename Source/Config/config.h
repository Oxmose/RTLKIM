/*******************************************************************************
 *
 * File: config.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 09/03/2018
 *
 * Version: 1.0
 *
 * Kernel configuration definitions
 ******************************************************************************/

#ifndef __CONFIG_H_
#define __CONFIG_H_

#define KERNEL_STACK_SIZE 16384 /* DO NOT FORGET TO MODIFY IN LOADER.S */
#define MAX_CPU_COUNT     1

#define ENABLE_SMP        0
#define ENABLE_VESA       1
#define ENABLE_ATA        0
#define ENABLE_GUI        1
#define ENABLE_MOUSE      1

/* Screen settings */
#define MAX_SUPPORTED_HEIGHT 1080
#define MAX_SUPPORTED_WIDTH  1100
#define MAX_SUPPORTED_BPP    32


#endif /* __CONFIG_H_ */