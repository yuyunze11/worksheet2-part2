#ifndef INCLUDE_FRAME_BUFFER_H
#define INCLUDE_FRAME_BUFFER_H

#include "types.h"

/* Frame buffer colors */
#define FB_BLACK         0
#define FB_BLUE          1
#define FB_GREEN         2
#define FB_CYAN          3
#define FB_RED           4
#define FB_MAGENTA       5
#define FB_BROWN         6
#define FB_LIGHT_GREY    7
#define FB_DARK_GREY     8
#define FB_LIGHT_BLUE    9
#define FB_LIGHT_GREEN   10
#define FB_LIGHT_CYAN    11
#define FB_LIGHT_RED     12
#define FB_LIGHT_MAGENTA 13
#define FB_LIGHT_BROWN   14
#define FB_WHITE         15

void fb_move_cursor(unsigned short pos);
void fb_write_cell(unsigned int i, char c, unsigned char fg, unsigned char bg);
void fb_write_char(char c);
void fb_write_string(const char* str);
void fb_backspace(void);
void fb_newline(void);
void fb_clear(void);
void fb_write_hex(u8int value);

#endif