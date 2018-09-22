/*******************************************************************************
 *
 * File: graphic.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 04/01/2018
 *
 * Version: 1.0
 *
 * Graphic drivers abtraction
 ******************************************************************************/


#include <Lib/stdint.h>      /* Generic int types */
#include <Lib/stddef.h>      /* OS_RETURN_E */

#include <Drivers/vga_text.h> /* VGA drivers */

/* Header file */
#include <Drivers/graphic.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/
uint8_t selected_driver = VGA_DRIVER_SELECTED;

#if 0
static const uint32_t vga_color_table[16] = {
    0xFF000000,
    0xFF0000AA,
    0xFF00AA00,
    0xFF00AAAA,
    0xFFAA0000,
    0xFFAA00AA,
    0xFFAA5500,
    0xFFAAAAAA,
    0xFF555555,
    0xFF5555FF,
    0xFF55FF55,
    0xFF55FFFF,
    0xFFFF5555,
    0xFFFF55FF,
    0xFFFFFF55,
    0xFFFFFFFF
};
#endif
/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

void set_selected_driver(const GRAPHIC_DRIVER_E sel)
{
	selected_driver = sel;
}

void clear_screen(void)
{
	vga_clear_screen();
}


OS_RETURN_E put_cursor_at(const uint32_t line, const uint32_t column)
{
	return vga_put_cursor_at(line, column);
}

OS_RETURN_E save_cursor(cursor_t* buffer)
{
	return vga_save_cursor(buffer);
}

OS_RETURN_E restore_cursor(const cursor_t buffer)
{
	return vga_restore_cursor(buffer);
}

void scroll(const SCROLL_DIRECTION_E direction,
            const uint32_t lines_count)
{
	vga_scroll(direction, lines_count);
}

void set_color_scheme(colorscheme_t color_scheme)
{
	vga_set_color_scheme(color_scheme);
}

OS_RETURN_E save_color_scheme(colorscheme_t* buffer)
{
	return vga_save_color_scheme(buffer);
}

void screen_put_string(const char* str)
{
	vga_put_string(str);
}

void screen_put_char(const char character)
{
    vga_put_char(character);
}

void console_write_keyboard(const char* str, const uint32_t len)
{
	vga_console_write_keyboard(str, len);
}
