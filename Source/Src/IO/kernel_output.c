/***************************************************************************//**
 * @file kernel_output.c
 * 
 * @see kernel_output.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 15/12/2017
 *
 * @version 2.0
 *
 * @brief Kernel's output methods.
 * 
 * @details Simple output functions to print messages to screen. These are 
 * really basic output too allow early kernel boot output and debug. These 
 * functions can be used in interrupts handlers since no lock is required to use
 * them. This also makes them non thread safe.
 * 
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <Lib/string.h>       /* memset, strlen */
#include <Lib/stdlib.h>       /* uitoa, itoa */
#include <BSP/serial.h>   /* Serial driver */
#include <IO/graphic.h>       /* graphic_save_color_scheme, set_color_sheme,
                               * graphic_put_char, graphic_put_string */

/* RTLK configuration file */
#include <config.h>

/* Header file */
#include <IO/kernel_output.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/** @brief Stores the current output type. */
static output_t current_output;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Converts a string to upper case characters.
 * 
 * @details Transforms all lowercase character of a NULL terminated string to 
 * uppercase characters.
 * 
 * @param[in,out] string The string to tranform.
 */
static void toupper(char* string)
{
    /* For each character of the string */
    while(*string != 0)
    {
        /* If the character is lowercase, makes it uppercase */
        if(*string > 96 && *string < 123)
        {
            *string = *string - 32;
        }
        ++string;
    }
}

/**
 * @brief Converts a string to upper case characters.
 * 
 * @details Transforms all uppercase character of a NULL terminated string to 
 * lowercase characters.
 * 
 * @param[in,out] string The string to tranform.
 */
static void tolower(char* string)
{
    /* For each character of the string */
    while(*string != 0)
    {
        /* If the character is uppercase, makes it lowercase */
        if(*string > 64 && *string < 91)
        {
            *string = *string + 32;
        }
        ++string;
    }
}

/**
 * @brief Prints a formated string.
 * 
 * @details Prints a formated string to the output and managing the formated 
 * string arguments.
 * 
 * @param[in] str The formated string to output.
 * @param[in] args The arguments to use with the formated string.
 */
static void kprint_fmt(const char* str, __builtin_va_list args)
{
    uint32_t i;
    int32_t  char_val;
    char*    args_value;
    char     tmp[32];
    int32_t  str_size;
    uint32_t offset;
    char     char_padding = ' ';
    int32_t  padding = -1;

	for(i = 0; i < strlen(str); ++i)
	{
		if(str[i] == '%')
		{
            offset = 1;
            /* Search for padding */
            if((str[i + offset] >= 48 && str[i + offset] <= 57))
            {
                char_padding = str[i + offset];
                ++offset;

                /* Search for padding size */
                if((str[i + offset] >= 48 && str[i + offset] <= 57))
                {
                    padding = str[i + offset] - 48;
                    ++offset;
                }
                else
                {
                    padding = 0;
                }
            }

            /* Search for format */
			switch(str[i + offset])
			{
				case 's':
					args_value = __builtin_va_arg(args, char*);
					current_output.puts(args_value);
                    i += offset;
					continue;
                case 'i':
				case 'd':

					char_val = __builtin_va_arg(args, int32_t);
					memset(tmp, 0, sizeof(tmp));
					itoa(char_val, tmp, 10);
                    str_size = strlen(tmp);
                    while(padding > str_size)
                    {
                        current_output.putc(char_padding);
                        --padding;
                    }
					current_output.puts(tmp);
                    i += offset;
					continue;
                case 'u':
                    char_val = __builtin_va_arg(args, uint32_t);
                    memset(tmp, 0, sizeof(tmp));
                    uitoa(char_val, tmp, 10);
                    str_size = strlen(tmp);
                    while(padding > str_size)
                    {
                        current_output.putc(char_padding);
                        --padding;
                    }
                    current_output.puts(tmp);
                    i += offset;
					continue;
				case 'x':
					char_val = __builtin_va_arg(args, uint32_t);
					memset(tmp, 0, sizeof(tmp));
					uitoa(char_val, tmp, 16);
                    str_size = strlen(tmp);
                    while(padding > str_size)
                    {
                        current_output.putc(char_padding);
                        --padding;
                    }
                    tolower(tmp);
					current_output.puts(tmp);
                    i += offset;
					continue;
                case 'X':
                    char_val = __builtin_va_arg(args, uint32_t);
                    memset(tmp, 0, sizeof(tmp));
                    uitoa(char_val, tmp, 16);
                    str_size = strlen(tmp);
                    while(padding > str_size)
                    {
                        current_output.putc(char_padding);
                        --padding;
                    }
                    toupper(tmp);
                    current_output.puts(tmp);
                    i += offset;
					continue;
				case 'c':
					tmp[0] = (char)
                        ((__builtin_va_arg(args, int32_t) & 0x000000FF));
					current_output.putc(tmp[0]);
                    i += offset;
					continue;
				default:
                    ++i;
                    continue;
			}
		}
        else
        {
            padding = -1;
			current_output.putc(str[i]);
		}
	}
}

/**
 * @brief Prints a formated string to serial output.
 * 
 * @details Prints a formated string to the serial output and managing the 
 * formated string arguments.
 * 
 * @param[in] str The formated string to output.
 * @param[in] args The arguments to use with the formated string.
 */
static void kprint_fmt_serial(const char* str, __builtin_va_list args)
{
    uint32_t i;
    int32_t  char_val;
    char*    args_value;
    char     tmp[32];
    int32_t  str_size;
    uint32_t offset;
    char     char_padding = ' ';
    int32_t  padding = -1;

	for(i = 0; i < strlen(str); ++i)
	{
		if(str[i] == '%')
		{
            offset = 1;
            /* Search for padding */
            if((str[i + offset] >= 48 && str[i + offset] <= 57))
            {
                char_padding = str[i + offset];
                ++offset;

                /* Search for padding size */
                if((str[i + offset] >= 48 && str[i + offset] <= 57))
                {
                    padding = str[i + offset] - 48;
                    ++offset;
                }
                else
                {
                    padding = 0;
                }
            }

            /* Search for format */
			switch(str[i + offset])
			{
				case 's':
					args_value = __builtin_va_arg(args, char*);
					serial_put_string(args_value);
                    i += offset;
					continue;
                case 'i':
				case 'd':

					char_val = __builtin_va_arg(args, int32_t);
					memset(tmp, 0, sizeof(tmp));
					itoa(char_val, tmp, 10);
                    str_size = strlen(tmp);
                    while(padding > str_size)
                    {
                        serial_put_char(char_padding);
                        --padding;
                    }
					serial_put_string(tmp);
                    i += offset;
					continue;
                case 'u':
                    char_val = __builtin_va_arg(args, uint32_t);
                    memset(tmp, 0, sizeof(tmp));
                    uitoa(char_val, tmp, 10);
                    str_size = strlen(tmp);
                    while(padding > str_size)
                    {
                        serial_put_char(char_padding);
                        --padding;
                    }
                    serial_put_string(tmp);
                    i += offset;
					continue;
				case 'x':
					char_val = __builtin_va_arg(args, uint32_t);
					memset(tmp, 0, sizeof(tmp));
					uitoa(char_val, tmp, 16);
                    str_size = strlen(tmp);
                    while(padding > str_size)
                    {
                        serial_put_char(char_padding);
                        --padding;
                    }
                    tolower(tmp);
					serial_put_string(tmp);
                    i += offset;
					continue;
                case 'X':
                    char_val = __builtin_va_arg(args, uint32_t);
                    memset(tmp, 0, sizeof(tmp));
                    uitoa(char_val, tmp, 16);
                    str_size = strlen(tmp);
                    while(padding > str_size)
                    {
                        serial_put_char(char_padding);
                        --padding;
                    }
                    toupper(tmp);
                    serial_put_string(tmp);
                    i += offset;
					continue;
				case 'c':
					tmp[0] = (char)
                        ((__builtin_va_arg(args, int32_t) & 0x000000FF));
					serial_put_char(tmp[0]);
                    i += offset;
					continue;
				default:
                    ++i;
                    continue;
			}
		}
        else
        {
            padding = -1;
			serial_put_char(str[i]);
		}
	}
}

/**
 * @brief Prints the tag for kernel output functions.
 * 
 * @details Prints the tag for kernel output functions.
 * 
 * @param[in] fmt The formated string to print.
 * @param[in] ... The associated arguments to the formated string.
 */
static void tag_printf(const char* fmt, ...)
{
    __builtin_va_list args;

    if(fmt == NULL)
    {
        return;
    }
    /* Prtinf format string */
    __builtin_va_start(args, fmt);
    kprint_fmt(fmt, args);
    __builtin_va_end(args);
}

void kernel_printf(const char* fmt, ...)
{
    __builtin_va_list args;

    if(fmt == NULL)
    {
        return;
    }

    /* Prtinf format string */
    __builtin_va_start(args, fmt);
    current_output.putc = graphic_put_char;
    current_output.puts = graphic_put_string;
    kprint_fmt(fmt, args);
    __builtin_va_end(args);
}

void kernel_error(const char* fmt, ...)
{
    __builtin_va_list args;
    colorscheme_t     buffer;
    colorscheme_t     new_scheme;

    if(fmt == NULL)
    {
        return;
    }

    new_scheme.foreground = FG_RED;
    new_scheme.background = BG_BLACK;
    new_scheme.vga_color  = 1;

    current_output.putc = graphic_put_char;
    current_output.puts = graphic_put_string;

    /* No need to test return value */
    graphic_save_color_scheme(&buffer);

    /* Set REG on BLACK color scheme */
    graphic_set_color_scheme(new_scheme);

    /* Print tag */
    tag_printf("[ERROR] ");

    /* Restore original screen color scheme */
    graphic_set_color_scheme(buffer);

    /* Printf format string */
    __builtin_va_start(args, fmt);
    kprint_fmt(fmt, args);
    __builtin_va_end(args);
}

void kernel_success(const char* fmt, ...)
{
    __builtin_va_list    args;
    colorscheme_t        buffer;
    colorscheme_t        new_scheme;

    if(fmt == NULL)
    {
        return;
    }

    new_scheme.foreground = FG_GREEN;
    new_scheme.background = BG_BLACK;
    new_scheme.vga_color  = 1;

    current_output.putc = graphic_put_char;
    current_output.puts = graphic_put_string;

    /* No need to test return value */
    graphic_save_color_scheme(&buffer);

    /* Set REG on BLACK color scheme */
    graphic_set_color_scheme(new_scheme);

    /* Print tag */
    tag_printf("[OK] ");

    /* Restore original screen color scheme */
    graphic_set_color_scheme(buffer);

    /* Printf format string */
    __builtin_va_start(args, fmt);
    kprint_fmt(fmt, args);
    __builtin_va_end(args);
}

void kernel_info(const char* fmt, ...)
{
    __builtin_va_list    args;
    colorscheme_t        buffer;
    colorscheme_t        new_scheme;

    if(fmt == NULL)
    {
        return;
    }

    new_scheme.foreground = FG_CYAN;
    new_scheme.background = BG_BLACK;
    new_scheme.vga_color  = 1;

    current_output.putc = graphic_put_char;
    current_output.puts = graphic_put_string;

    /* No need to test return value */
    graphic_save_color_scheme(&buffer);

    /* Set REG on BLACK color scheme */
    graphic_set_color_scheme(new_scheme);

    /* Print tag */
    tag_printf("[INFO] ");

    /* Restore original screen color scheme */
    graphic_set_color_scheme(buffer);

    /* Printf format string */
    __builtin_va_start(args, fmt);
    kprint_fmt(fmt, args);
    __builtin_va_end(args);
}

void kernel_debug(const char* fmt, ...)
{
    __builtin_va_list    args;
    colorscheme_t        buffer;
    colorscheme_t        new_scheme;

    if(fmt == NULL)
    {
        return;
    }

    new_scheme.foreground = FG_YELLOW;
    new_scheme.background = BG_BLACK;
    new_scheme.vga_color  = 1;

    current_output.putc = graphic_put_char;
    current_output.puts = graphic_put_string;

    /* No need to test return value */
    graphic_save_color_scheme(&buffer);

    /* Set REG on BLACK color scheme */
    graphic_set_color_scheme(new_scheme);

    /* Print tag */
    tag_printf("[DEBUG] ");

    /* Restore original screen color scheme */
    graphic_set_color_scheme(buffer);

    /* Printf format string */
    __builtin_va_start(args, fmt);
    kprint_fmt(fmt, args);
    __builtin_va_end(args);
}

void kernel_serial_debug(const char* fmt, ...)
{
    __builtin_va_list args;

    if(fmt == NULL)
    {
        return;
    }

    __builtin_va_start(args, fmt);
    kprint_fmt_serial("[DEBUG] ", args);

    kprint_fmt_serial(fmt, args);
    __builtin_va_end(args);

}

/*******************************************************************************
 * This function should only be called when the kernel is fully Initialized
 ******************************************************************************/
void kernel_doprint(const char* str, __builtin_va_list args)
{
    if(str == NULL)
    {
        return;
    }

    current_output.putc = graphic_put_char;
    current_output.puts = graphic_put_string;
    kprint_fmt(str, args);
}
