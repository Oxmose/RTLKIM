/*******************************************************************************
 * File: config.h
 *
 * Original author: Alexy Torres Aurora Dugo
 *
 * Created: 09/03/2018
 * 
 * Last author: Alexy Torres Aurora Dugo
 * 
 * Last modified: 21/09/2018
 *
 * Version: 2.0
 *
 * Kernel configuration's definitions. This file stores the different settings 
 * used when compiling RTLK.
 ******************************************************************************/

#ifndef __CONFIG_H_
#define __CONFIG_H_

#define KERNEL_STACK_SIZE 16384 /* DO NOT FORGET TO MODIFY IN LOADER.S */
#define MAX_CPU_COUNT     1

#define ENABLE_SMP        0
#define ENABLE_VESA       0
#define ENABLE_ATA        0
#define ENABLE_GUI        0
#define ENABLE_MOUSE      0

/* Screen settings */
#define MAX_SUPPORTED_HEIGHT 1080
#define MAX_SUPPORTED_WIDTH  1100
#define MAX_SUPPORTED_BPP    32

#endif /* __CONFIG_H_ */