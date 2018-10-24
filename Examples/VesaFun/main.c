#include <Drivers/vesa.h>
#include <Lib/stdio.h>
#include <Drivers/ata_pio.h>
#include <Memory/kheap.h>


int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    uint32_t img_width = 1920;
    uint32_t img_height = 1080;

    uint8_t* buffer = kmalloc(img_width * sizeof(uint32_t));

    uint32_t width;
    uint32_t height;
    uint32_t i;
    uint32_t j;

    /* Get settings */
    width  = vesa_get_screen_width();
    height = vesa_get_screen_height();

    uint32_t sector = 0;
    ata_pio_device_t dev;
    dev.port = PRIMARY_PORT;
    dev.type = MASTER;

    OS_RETURN_E err;

    /* Draw the image */
    for(i = 0; i < MIN(height, img_height); ++i)
    {
        /* Copy the buffer */
        uint32_t toread = img_width * sizeof(uint32_t);
        while(toread)
        {
            uint32_t pass =  MIN(ATA_PIO_SECTOR_SIZE, toread);
            err = ata_pio_read_sector(dev, sector++, buffer + (img_width * sizeof(uint32_t) - toread), pass);
            if(err != OS_NO_ERR)
            {
                kernel_error("Could not load buffer");
            }
            toread -= pass;
        }

        /* Write pixels */
        uint32_t index = 0;
        for(j = 0; j < width; ++j)
        {
            vesa_draw_pixel(j, i, 0xFF, buffer[index], buffer[index+1], buffer[index+2]);
            index += 4;
        }
    }

    /* Set transparent characters */
    cursor_t cursor;
    cursor.x = width / 2 - 14 * 8;
    cursor.y = height / 2 - 8;

    vesa_restore_cursor(cursor);
    vesa_set_transparent_char(1);

    /* Say hello */
    printf("Hello from VesaFun example!");

    while(1);
}