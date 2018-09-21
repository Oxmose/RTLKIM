/*******************************************************************************
 *
 * File: memrchr.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 03/10/2017
 *
 * Version: 1.0
 *
 * memrchr function. To be used with string.h header.
 *
 ******************************************************************************/

#include <Lib/stddef.h> /* size_t */

/* Header file */
#include <Lib/string.h>

void *memrchr(const void *s, int c, size_t n)
{
    const unsigned char *sp = (const unsigned char *)s + n - 1;

    while (n--) {
        if (*sp == (unsigned char)c)
            return (void *)sp;
        sp--;
    }

    return NULL;
}
