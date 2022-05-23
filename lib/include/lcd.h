/*
 * I2C LCD driver
 *
 * Copyright (c) 2022 Matt Liss
 * BSD-3-Clause
 */
#ifndef _LCD_H_
#define _LCD_H_

#include <stdint.h>


// user lcd config
#ifndef LCD_WIDTH
#error "Must define LCD_WIDTH"
#endif
#ifndef LCD_HEIGHT
#error "Must define LCD_HEIGHT"
#endif
#ifndef LCD_I2C_ADDR
#define LCD_I2C_ADDR            0x27
#endif
#ifndef LCD_EN
#define LCD_EN                  2
#endif
#ifndef LCD_RW
#define LCD_RW                  1
#endif
#ifndef LCD_RS
#define LCD_RS                  0
#endif
#ifndef LCD_D4
#define LCD_D4                  4
#endif
#ifndef LCD_D5
#define LCD_D5                  5
#endif
#ifndef LCD_D6
#define LCD_D6                  6
#endif
#ifndef LCD_D7
#define LCD_D7                  7
#endif
#ifndef LCD_BACKLIGHT
#define LCD_BACKLIGHT           3
#endif


// lcd config definitions
#define LCD_CFG_BACKLIGHT_ON    0x08
#define LCD_CFG_DISPLAY_ON      0x04
#define LCD_CFG_CURSOR_ON       0x02
#define LCD_CFG_CURSOR_BLINK    0x01


// lcd special characters
#define LCD_CHAR_RARR           0x7e
#define LCD_CHAR_LARR           0x7f
#define LCD_CHAR_DEG            0xdf

#define LCD_CHAR_TCURS          0x01
#define LCD_CHAR_MCURS          0x02
#define LCD_CHAR_BCURS          0x03

#define LCD_CHAR_BATT_0         0x01
#define LCD_CHAR_BATT_20        0x02
#define LCD_CHAR_BATT_40        0x03
#define LCD_CHAR_BATT_60        0x04
#define LCD_CHAR_BATT_80        0x05
#define LCD_CHAR_BATT_100       0x06

typedef enum lcd_cgset {
    LCD_CGSET_MENU,
    LCD_CGSET_BATT,
    LCD_CGSET_MAX
} lcd_cgset;

void lcd_init (void);
void lcd_write_cgram (lcd_cgset cgset);
void lcd_config (uint8_t conf);
void lcd_clr (void);
void lcd_home (void);
void lcd_move (uint8_t line, uint8_t chr);
void lcd_putch (char c);
void lcd_putb (char *s, uint8_t len);
void lcd_puts (char *s);


#ifdef LCD_INT_PUT_FUNCTIONS
void lcd_puti (int32_t num, uint8_t digits);
void lcd_putui (uint32_t num, uint8_t digits);
#endif


#endif // _LCD_H_
