/***************************************************************************//**
 * @file serial.c
 *
 * @see serial.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 03/05/2019
 *
 * @version 1.0
 *
 * @brief PL011 UART communication driver.
 *
 * @details Serial communication driver. Initializes the serial ports as in and
 * output. The serial can be used to output data or communicate with other
 * prepherals that support this communication method.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <Lib/stddef.h>    /* OS_RETURN_E */
#include <Lib/stdint.h>    /* Generic int types */
#include <Lib/string.h>    /* strlen */

/* UTK configuration file */
#include <config.h>

/* Header file */
#include <BSP/serial.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/**
 * @brief Serial text driver instance.
 */
kernel_graphic_driver_t serial_text_driver = {
    .clear_screen = serial_clear_screen,
    .put_cursor_at = serial_put_cursor_at,
    .save_cursor = serial_save_cursor,
    .restore_cursor = serial_restore_cursor,
    .scroll = serial_scroll,
    .set_color_scheme = serial_set_color_scheme,
    .save_color_scheme = serial_save_color_scheme,
    .put_string = serial_put_string,
    .put_char = serial_put_char,
    .console_write_keyboard = serial_console_write_keyboard
};

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Sets line parameters for the desired port.
 *
 * @details Sets line parameters for the desired port.
 *
 * @param[in] attr The settings for the port's line.
 * @param[in] com The port to set.
 *
 * @return OS_NO_ERR on success, no other value is returned.
 */
static OS_RETURN_E set_line(const uint8_t attr, const uint32_t com)
{
    /*TODO*/
    (void)attr;
    (void)com;

    return OS_ERR_NOT_SUPPORTED;
}

/**
 * @brief Sets buffer parameters for the desired port.
 *
 * @details Sets buffer parameters for the desired port.
 *
 * @param[in] attr The settings for the port's buffer.
 * @param[in] com The port to set.
 *
 * @return OS_NO_ERR on success, no other value is returned.
 */
static OS_RETURN_E set_buffer(const uint8_t attr, const uint32_t com)
{
    /*TODO*/
    (void)attr;
    (void)com;

    return OS_ERR_NOT_SUPPORTED;
}

/**
 * @brief Sets the port's baudrate.
 *
 * @details Sets the port's baudrate.
 *
 * @param[in] rate The desired baudrate for the port.
 * @param[in] com The port to set.
 *
 * @return OS_NO_ERR on success, no other value is returned.
 */
static OS_RETURN_E set_baudrate(SERIAL_BAUDRATE_E rate, const uint32_t com)
{
    /*TODO*/
    (void)rate;
    (void)com;
    
    return OS_ERR_NOT_SUPPORTED;
}

OS_RETURN_E serial_init(void)
{
    /*TODO*/
    set_buffer(0, 0);
    set_baudrate(BAUDRATE_115200, 0);
    set_line(0, 0);
    return OS_ERR_NOT_SUPPORTED;
}

void serial_write(const uint32_t port, const uint8_t data)
{
    if(port != COM1 && port != COM2 && port != COM3 && port != COM4)
    {
        return;
    }

    if(data == '\n')
    {
        serial_write(port, '\r');
        *(volatile uint32_t*)(port) = '\n';
    }
    else
    {
        *(volatile uint32_t*)(port) = data;
    }
}


void serial_clear_screen(void)
{
    uint8_t i;
    /* On 80x25 screen, just print 25 line feed. */
    for(i = 0; i < 25; ++i)
    {
        serial_write(SERIAL_DEBUG_PORT, '\n');
    }
}

OS_RETURN_E serial_put_cursor_at(const uint32_t line, const uint32_t column)
{
    (void)line;
    (void)column;
    /* Nothing to do here */
    return OS_ERR_NOT_SUPPORTED;
}

OS_RETURN_E serial_save_cursor(cursor_t* buffer)
{
    (void)buffer;
    /* Nothing to do here */
    return OS_ERR_NOT_SUPPORTED;
}

OS_RETURN_E serial_restore_cursor(const cursor_t buffer)
{
    (void)buffer;
    /* Nothing to do here */
    return OS_ERR_NOT_SUPPORTED;
}

void serial_scroll(const SCROLL_DIRECTION_E direction,
                   const uint32_t lines_count)
{
    uint32_t i;
    if(direction == SCROLL_DOWN)
    {
        /* Just print lines_count line feed. */
        for(i = 0; i < lines_count; ++i)
        {
            serial_write(SERIAL_DEBUG_PORT, '\n');
        }
    }    
}

void serial_set_color_scheme(const colorscheme_t color_scheme)
{
    (void)color_scheme;
}

OS_RETURN_E serial_save_color_scheme(colorscheme_t* buffer)
{
    (void)buffer;

    return OS_ERR_NOT_SUPPORTED;
}

void serial_console_write_keyboard(const char* str, const uint32_t len)
{
    uint32_t i;
    for(i = 0; i < len; ++i)
    {
        serial_write(SERIAL_DEBUG_PORT, str[i]);
    }
}

uint8_t serial_read(const uint32_t port)
{
    /*TODO*/
    (void)port;
    return 0;
}

void serial_put_string(const char* string)
{
    uint32_t i;
    for(i = 0; i < strlen(string); ++i)
    {
        serial_write(SERIAL_DEBUG_PORT, string[i]);
    }
}

void serial_put_char(const char character)
{
    serial_write(SERIAL_DEBUG_PORT, character);
}
