/***************************************************************************//**
 * @file serial.c
 * 
 * @see serial.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 25/12/2017
 *
 * @version 1.0
 *
 * @brief Serial communication driver.
 * 
 * @details Serial communication driver. Initializes the serial ports as in and
 * output. The serial can be used to output data or communicate with other 
 * prepherals that support this communication method. Only COM1 to COM4 are
 * supported by this driver.
 * 
 * @copyright Alexy Torres Aurora Dugo
 * 
 * @warning Only COM1 and COM2 are initialized for input.
 ******************************************************************************/

#include <Lib/stddef.h>    /* OS_RETURN_E */
#include <Lib/stdint.h>    /* Generic int types */
#include <Lib/string.h>    /* strlen */
#include <Cpu/cpu.h>       /* outb, inb */
#include <Sync/critical.h> /* ENTER_CRITICAL, EXIT_CRITICAL */

/* RTLK configuration file */
#include <config.h>

/* Header file */
#include <Drivers/serial.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/** @brief Stores the serial initialization state. */
static uint8_t serial_init_done = 0;

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
    cpu_outb(attr, SERIAL_LINE_COMMAND_PORT(com));

    return OS_NO_ERR;
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
    cpu_outb(attr, SERIAL_FIFO_COMMAND_PORT(com));

    return OS_NO_ERR;
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
    cpu_outb(SERIAL_DLAB_ENABLED, SERIAL_LINE_COMMAND_PORT(com));
    cpu_outb((rate >> 8) & 0x00FF, SERIAL_DATA_PORT(com));
    cpu_outb(rate & 0x00FF, SERIAL_DATA_PORT_2(com));

    return OS_NO_ERR;
}

OS_RETURN_E serial_init(void)
{
    OS_RETURN_E err;
    uint8_t i;

    #if SERIAL_KERNEL_DEBUG == 1
    kernel_serial_debug("Serial Initialization start\n");
    #endif

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
            cpu_outb(0x01, SERIAL_DATA_PORT_2(com));
        }
        else
        {
            cpu_outb(0x00, SERIAL_DATA_PORT_2(com));
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
        cpu_outb(0x0B, SERIAL_MODEM_COMMAND_PORT(com));
    }

    serial_init_done = 1;

    #if SERIAL_KERNEL_DEBUG == 1
    kernel_serial_debug("Serial Initialization end\n");
    #endif

    return err;
}

void serial_write(const uint32_t port, const uint8_t data)
{
    //uint32_t word;

    if(serial_init_done == 0)
    {
        return;
    }
    if(port != COM1 && port != COM2 && port != COM3 && port != COM4)
    {
        return;
    }

    //ENTER_CRITICAL(word);

    /* Wait for empty transmit */
    while((SERIAL_LINE_STATUS_PORT(port) & 0x20) == 0)
    {}

    if(data == '\n')
    {
        serial_write(port, '\r');
        cpu_outb('\n', port);
    }
    else
    {
        cpu_outb(data, port);
    }

    while((SERIAL_LINE_STATUS_PORT(port) & 0x20) == 0)
    {}

    //EXIT_CRITICAL(word);
}

uint8_t serial_read(const uint32_t port)
{
    //uint32_t word;
    //ENTER_CRITICAL(word);
    
    /* Wait for data to be received */
    while (serial_received(port) == 0);

    /* Read available data on port */
    uint8_t val = cpu_inb(SERIAL_DATA_PORT(port));

    //EXIT_CRITICAL(word);
    return val;
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
    return cpu_inb(SERIAL_LINE_STATUS_PORT(port)) & 0x01;
}
