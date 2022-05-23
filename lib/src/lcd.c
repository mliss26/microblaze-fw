/*
 * I2C LCD display driver
 *
 * Copyright (c) 2022 Matt Liss
 * BSD-3-Clause
 */

#include "lcd.h"
#include "twi.h"
#include "gcnt.h"
#include "util.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>


#define LCD_MODE_COMMAND    0
#define LCD_MODE_DATA       (1 << LCD_RS)
#define I2C_M_WR            0 // i2c-dev write uses no flags


/*
 * CGRAM data for custom character bitmaps
 *
 * Total size must be a multiple of 8 bytes. Each character map is composed of
 * the 5 least significant bits of 7 consecutive bytes. The eighth byte in
 * each character map should be all zeros. Max of 8 custom charaters per set.
 */
static uint8_t cgdata_menu[] = {
    0x00, 0x04, 0x02, 0x1f, 0x12, 0x14, 0x10, 0x00, // top cursor
    0x10, 0x14, 0x12, 0x1f, 0x12, 0x14, 0x10, 0x00, // middle cursor
    0x10, 0x14, 0x12, 0x1f, 0x02, 0x04, 0x00, 0x00, // bottom cursor
};

static uint8_t cgdata_batt[] = {
    0x0e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1f, 0x00, // battery 0%
    0x0e, 0x11, 0x11, 0x11, 0x11, 0x1f, 0x1f, 0x00, // battery 20%
    0x0e, 0x11, 0x11, 0x11, 0x1f, 0x1f, 0x1f, 0x00, // battery 40%
    0x0e, 0x11, 0x11, 0x1f, 0x1f, 0x1f, 0x1f, 0x00, // battery 60%
    0x0e, 0x11, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x00, // battery 80%
    0x0e, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x00, // battery 100%
};

// state of backlight
static bool backlight_on = true;

// location of cursor
static struct lcd_position {
    uint8_t ln;
    uint8_t ch;
} pos;


/*
 * Write a nibble to the lcd
 *
 * Nibble value includes masks for EN, RS and RW bits
 *
 * Returns true if LCD Acked transaction
 */
static bool lcd_write (uint8_t value)
{
    twi_write(LCD_I2C_ADDR, &value, sizeof(value), NULL);

    return true;
}

static uint8_t lcd_map_nibble (uint8_t value)
{
    uint8_t mapped = 0;

    if (value & 0x1)
        mapped |= (1 << LCD_D4);
    if (value & 0x2)
        mapped |= (1 << LCD_D5);
    if (value & 0x4)
        mapped |= (1 << LCD_D6);
    if (value & 0x8)
        mapped |= (1 << LCD_D7);

    return mapped;
}

static bool lcd_write_nibble (uint8_t value, uint8_t mode)
{
    uint8_t nibble;

    if (backlight_on)
        mode |= (1 << LCD_BACKLIGHT);

    nibble = lcd_map_nibble(value) | mode;

    if (!lcd_write(nibble | (1 << LCD_EN)))
        return false;
    if (!lcd_write(nibble))
        return false;

    return true;
}

/*
 * Write a byte to the lcd
 *
 * Mode chooses between command and data
 */
static bool lcd_write_byte (uint8_t value, uint8_t mode)
{
    // upper nibble
    if (!lcd_write_nibble((value >> 4), mode))
        return false;

    // lower nibble
    if (!lcd_write_nibble((value & 0xF), mode))
        return false;

    return true;
}

/*
 * Write a command byte to the lcd
 */
static void lcd_write_cmd (uint8_t cmd)
{
    lcd_write_byte(cmd, LCD_MODE_COMMAND);
}

/*
 * Write a byte of character data to the cursor's location
 */
static void lcd_write_data (uint8_t data)
{
    lcd_write_byte(data, LCD_MODE_DATA);
}

/*
 * Write the custom character bitmaps to CGRAM
 *
 * First character code is 1 so 0 can still be treated as NULL/end of string.
 */
void lcd_write_cgram (lcd_cgset cgset)
{
    uint8_t i;

    // set CGRAM address to start of character 1
    lcd_write_cmd(0x48);

    // write character map
    switch (cgset) {
        case LCD_CGSET_MENU:
            for (i = 0; i < sizeof(cgdata_menu); ++i)
                lcd_write_data(cgdata_menu[i]);
            break;
        case LCD_CGSET_BATT:
            for (i = 0; i < sizeof(cgdata_batt); ++i)
                lcd_write_data(cgdata_batt[i]);
            break;
        default:
            break;
    }
}

/*
 * Initialize the io pins controlling the lcd and configure
 * the lcd with default settings. This MUST be called prior
 * to any of the other functions.
 */
void lcd_init (void)
{
    // LCD initialization specified by controller doc
    delay_us(50000);

    // set 4 bit mode with special function set sequence
    lcd_write_nibble(0x3, LCD_MODE_COMMAND);
    delay_us(4500); // min 4.1 ms

    lcd_write_nibble(0x3, LCD_MODE_COMMAND);
    delay_us(150); // min 100 us

    lcd_write_nibble(0x3, LCD_MODE_COMMAND);
    delay_us(150); // min 100 us

    lcd_write_nibble(0x2, LCD_MODE_COMMAND);
    delay_us(150); // min 100 us

    lcd_write_byte(0x28, LCD_MODE_COMMAND);   // 4-bit interface, 2 line mode
    delay_us(60);

    lcd_write_byte(0x0c, LCD_MODE_COMMAND);   // display on

    lcd_write_byte(0x01, LCD_MODE_COMMAND);   // display clr
    delay_us(2000);

    lcd_write_byte(0x06, LCD_MODE_COMMAND);   // left to right increment
    delay_us(100);

    lcd_write_cgram(LCD_CGSET_MENU);      // write custom character bitmaps
}

/*
 * Configure the lcd screen.  The conf value may be a bitwise or
 * of any of the following values:
 *  LCD_DISPLAY_ON     Turn on the lcd display
 *  LCD_CURSOR_ON      Turn on the cursor (_)
 *  LCD_CURSOR_BLINK   Make the cursor blink
 *
 * If LCD_DISPLAY_ON is not set, the display will be turned off,
 * but its character memory will still take values to be displayed
 * the next time the display is turned on.
 */
void lcd_config (uint8_t conf)
{
    if (conf & LCD_CFG_BACKLIGHT_ON)
        backlight_on = true;
    else
        backlight_on = false;

    lcd_write_cmd(0x08 | conf);
}

/*
 * Clear the display by writing spaces over it all and
 * move the cursor to the home position (1,1).
 */
void lcd_clr (void)
{
    lcd_write_cmd(0x01);
    delay_us(2000);

    pos.ln = 0;
    pos.ch = 0;
}

/*
 * Move the lcd cursor to the home (1,1) position.
 */
void lcd_home (void)
{
    lcd_write_cmd(0x02);
    delay_us(2000);

    pos.ln = 0;
    pos.ch = 0;
}

/*
 * Move the lcd cursor to the desired location.  Valid
 * lines are [0,LCD_HEIGHT-1] and valid characters are [0,LCD_WIDTH-1]
 */
void lcd_move (uint8_t line, uint8_t chr)
{
    static uint8_t line_offset[] = { 0x80, 0xC0, 0x94, 0xD4 };
    pos.ln = line;
    pos.ch = chr;

    lcd_write_cmd(line_offset[pos.ln] + pos.ch);
}

/*
 *  Display a character on the lcd screen
 *
 *  Valid characters are all the ascii upper and lower case letters,
 *  digits, and symbols. Not all symbols are displayed in the same way
 *  as usual, but they still correspond to a character. A newline
 *  ('\n') will move to the next line of the display, and a carriage
 *  return ('\r') will move to the beginning of the current line.
 */
void lcd_putch (char c)
{
    if (c == '\n')
    {
        pos.ln++;
        pos.ch = 0;
        lcd_move(pos.ln, pos.ch);
    }
    else if (c == '\r')
    {
        pos.ch = 0;
        lcd_move(pos.ln, pos.ch);
    }
    else
    {
        lcd_write_data(c);
        pos.ch++;
    }
} /* end lcd_putch */

/*
 * Display a counted byte string on the lcd screen
 */
void lcd_putb (char *s, uint8_t len)
{
    while (len) {
        if (*s) {
            lcd_putch(*s);
        } else {
            lcd_putch(' ');
        }
        s++;
        len--;
    }
}

/*
 * Display a null terminated string on the lcd screen
 */
void lcd_puts (char *s)
{
    char *c = s;
    while (*c) {
        lcd_putch(*c);
        c++;
    }
}

#ifdef LCD_INT_PUT_FUNCTIONS

/* lcd_puti
 *  Display a signed integer of up to 32 bits on the lcd screen.
 *  If digits is 0, as many digits as needed will be displayed,
 *  without zero padding.  If digits is > 0, that many digits
 *  (up to 10) will be displayed, no matter what.
 */
void lcd_puti (int32_t num, uint8_t digits)
{
    uint32_t tmp;
    uint32_t pten = 1;
    uint8_t i, spaces = 0, write_zero = 0;

    if (digits == 0) {
        digits = 10;
    } else {
        if (digits > 10)
            digits = 10;

        //spaces = 11-digits;
        write_zero = 1;
    }

    for (i=1; i<digits; i++)
        pten *= 10;

    if (num < 0)
    {
        lcd_putch('-');
        num = -num;
    }

    for (i=0; i<digits; i++)
    {
        tmp = (num / pten) % 10;

        if (tmp)
            write_zero = 1;

        if (write_zero)
            lcd_putch(0x30 + tmp);

        pten /= 10;
    }
    if (!write_zero)
        lcd_putch(0x30);

    for (i=0; i<spaces; i++)
        lcd_putch(' ');
} // end lcd_puti

/* lcd_puti
 *  Display an unsigned integer of up to 32 bits on the lcd screen.
 *  If digits is 0, as many digits as needed will be displayed,
 *  without zero padding.  If digits is > 0, that many digits
 *  (up to 10) will be displayed, no matter what.
 */
void lcd_putui (uint32_t num, uint8_t digits)
{
    uint32_t tmp;
    uint32_t pten = 1;
    uint8_t i, spaces = 0, write_zero = 0;

    if (digits == 0) {
        digits = 10;
    } else {
        if (digits > 10)
            digits = 10;

        //spaces = 11-digits;
        write_zero = 1;
    }

    for (i=1; i<digits; i++)
        pten *= 10;

    for (i=0; i<digits; i++)
    {
        tmp = (num / pten) % 10;

        if (tmp)
            write_zero = 1;

        if (write_zero)
            lcd_putch(0x30 + tmp);

        pten /= 10;
    }
    if (!write_zero)
        lcd_putch(0x30);

    for (i=0; i<spaces; i++)
        lcd_putch(' ');
} // end lcd_putui

#endif // LCD_INT_PUT_FUNCTIONS
