/*******************************************************************************
 *
 * File: smp.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 27/03/2018
 *
 * Version: 1.0
 *
 * SMP implementation of the kernel. The different functions in this File
 * allow the systen to detect, initialize and manage CPU cores.
 ******************************************************************************/

#ifndef __SMP_H_
#define __SMP_H_

#include <Lib/stdint.h> /* Generic int types */
#include <Lib/stddef.h> /* OS_RETURN_E */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

 /**
  * @brief Initialize the system cores. The function will boot each cores and
  * set them to a normal protected mode state. Then each core will be running an
  * idle thread until the system is initialized.
  *
  * @return The success state or the error code.
  * - OS_NO_ERR is returned if no error is encountered.
  * - Other possible return code, returned by internally called functions.
  */
 OS_RETURN_E smp_init(void);

#endif /* __SMP_H_ */