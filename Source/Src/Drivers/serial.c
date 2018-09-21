/*******************************************************************************
 *
 * File: serial.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 25/12/2017
 *
 * Version: 1.0
 *
 * Serial driver for the kernel.
 ******************************************************************************/

#include <Lib/stddef.h>      /* OS_RETURN_E */
#include <Lib/stdint.h>      /* Generic int types */
#include <Lib/string.h>      /* strlen */
#include <Cpu/cpu.h>         /* outb, inb */

/* Header file */
#include <Drivers/serial.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

static uint8_t serial_init = 0;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Set line parameters for the desired port.
 *
 * @param attr The settings for the port's line.
 * @param com The port to set.
 * @returns OS_NO_ERR on success, en error is returned otherwise.
 */
static OS_RETURN_E set_line(const uint8_t attr, const uint32_t com)
{
    outb(attr, SERIAL_LINE_COMMAND_PORT(com));

    return OS_NO_ERR;
}

/* Set buffer parameters for the desired port.
 *
 * @param attr The settings for the port's buffer.
 * @param com The port to set.
 * @returns OS_NO_ERR on success, en error is returned otherwise.
 */
static OS_RETURN_E set_buffer(const uint8_t attr, const uint32_t com)
{
    outb(attr, SERIAL_FIFO_COMMAND_PORT(com));

    return OS_NO_ERR;
}

/* Set the port baudrate.
 *
 * @param rate The desired baudrate for the port.
 * @param com The port to set.
 * @returns OS_NO_ERR on success, en error is returned otherwise.
 */
static OS_RETURN_E set_baudrate(SERIAL_BAUDRATE_E rate, const uint32_t com)
{
    outb(SERIAL_DLAB_ENABLED, SERIAL_LINE_COMMAND_PORT(com));
    outb((rate >> 8) & 0x00FF, SERIAL_DATA_PORT(com));
    outb(rate & 0x00FF, SERIAL_DATA_PORT_2(com));

    return OS_NO_ERR;
}

OS_RETURN_E init_serial(void)
{
    OS_RETURN_E err;
    uint8_t i;

    /* Init all comm ports */
    for(i = 0; i < 4; ++i)
    {
        uint8_t attr;
        uint32_t com;

        if(i == 0)
        {
            com = SERIAL_COM1_BASE;
        }
        else if(i == 1)
        {
            com = SERIAL_COM2_BASE;
        }
        else if(i == 2)
        {
            com = SERIAL_COM3_BASE;
        }
        else if(i == 3)
        {
            com = SERIAL_COM4_BASE;
        }

        attr = SERIAL_DATA_LENGTH_8 | SERIAL_STOP_BIT_1;

        /* Enable interrupt on recv for COM1 and COM2 */
        if(com == SERIAL_COM1_BASE || com == SERIAL_COM2_BASE)
        {
            outb(0x01, SERIAL_DATA_PORT_2(com));
        }
        else
        {
            outb(0x00, SERIAL_DATA_PORT_2(com));
        }

        /* Init baud rate */
        err = set_baudrate(BAUDRATE_115200, com);
        if(err != OS_NO_ERR)
        {
            return err;
        }

        /* Configure the line */
        err = set_line(attr, com);
        if(err != OS_NO_ERR)
        {
            return err;
        }

        err = set_buffer(0xC0 | SERIAL_ENABLE_FIFO | SERIAL_CLEAR_RECV_FIFO |
                         SERIAL_CLEAR_SEND_FIFO | SERIAL_FIFO_DEPTH_14,
                         com);
        if(err != OS_NO_ERR)
        {
            return err;
        }

        /* Enable interrupt */
        outb(0x0B, SERIAL_MODEM_COMMAND_PORT(com));
    }

    serial_init = 1;

    return err;
}

void serial_write(const uint32_t port, const uint8_t data)
{
    if(serial_init == 0)
    {
        return;
    }
    if(port != COM1 && port != COM2 && port != COM3 && port != COM4)
    {
        return;
    }

    /* Wait for empty transmit */
    while((SERIAL_LINE_STATUS_PORT(port) & 0x20) == 0)
    {}

    if(data == '\n')
    {
        serial_write(port, '\r');
        outb('\n', port);
    }
    else
    {
        outb(data, port);
    }

    while((SERIAL_LINE_STATUS_PORT(port) & 0x20) == 0)
    {}
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

uint8_t serial_received(const uint32_t port)
{
    /* Read on LINE status port */
   return inb(SERIAL_LINE_STATUS_PORT(port)) & 0x01;
}

uint8_t read_serial(const uint32_t port)
{
    /* Wait for data to be received */
   while (serial_received(port) == 0);

   /* Read available data on port */
   return inb(SERIAL_DATA_PORT(port));
}
