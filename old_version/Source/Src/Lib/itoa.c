/*******************************************************************************
 *
 * File: iota.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 08/01/2018
 *
 * Version: 1.0
 *
 * itoa function. To be used with stdlib.h header.
 *
 ******************************************************************************/

#include <Lib/stddef.h> /* size_t */

/* Header include */
#include <Lib/stdlib.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

 void itoa(int64_t i, char* buf, uint32_t base)
 {
     /* If base is unknown just return */
     if (base > 16)
     {
         return;
     }

     /* Check sign */
     if (base == 10 && i < 0)
     {
        *buf++ = '-';
        i *= -1;
     }

     /* To the job */
     uitoa(i, buf, base);
 }
