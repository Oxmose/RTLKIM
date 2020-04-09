#include "img.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

int main()
{
    unsigned int i, j;
    FILE *fp;

    /* open a file to write */
    fp = fopen("img.bin", "wb");
    if (fp == NULL) {
             printf("Open Error\n");
             return 1;
    }

    for(i = 0; i < width; ++i)
    {
        for(j = 0; j < height; ++j)
        {
            char pixel[4];
            HEADER_PIXEL(header_data, pixel);
            if(fwrite(pixel , sizeof(char), 4, fp) != 4)
            {
                printf("Error %d\n", errno);
            }
        }
    }
    fclose(fp);
    printf("DONE\n");
    return 0;
}