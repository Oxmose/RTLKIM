/*******************************************************************************
 *
 * File: memccpy.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 03/10/2017
 *
 * Version: 1.0
 *
 * memccpy function. To be used with string.h header.
 *
 ******************************************************************************/

#include <Lib/stddef.h> /* size_t */

/* Header file */
#include <Lib/string.h>

void *memccpy(void *dst, const void *src, int c, size_t n)
{
    char *q = dst;
    const char *p = src;

    while (n--) {
        char ch;
        
        *q++ = ch = *p++;
        if (ch == (char)c)
            return q;
    }

    return NULL;
}
