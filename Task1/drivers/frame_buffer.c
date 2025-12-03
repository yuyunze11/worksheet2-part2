#include "io.h"
#include "frame_buffer.h"

#define FB_COMMAND_PORT         0x3D4
#define FB_DATA_PORT            0x3D5

#define FB_HIGH_BYTE_COMMAND    14
#define FB_LOW_BYTE_COMMAND     15

/* Frame buffer */
char *fb = (char *) 0x000B8000;

/* Current cursor position */
static u16int cursor_pos = 0;

void fb_move_cursor(u16int pos) {
    outb(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
    outb(FB_DATA_PORT,    ((pos >> 8) & 0x00FF));
    outb(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
    outb(FB_DATA_PORT,    pos & 0x00FF);
    cursor_pos = pos;
}

void fb_write_cell(unsigned int i, char c, unsigned char fg, unsigned char bg) {
    fb[i * 2] = c;
    fb[i * 2 + 1] = ((fg & 0x0F) << 4) | (bg & 0x0F);
}

void fb_write_char(char c) 
{
    if (c == '\n')
    {
        fb_newline();
        return;
    }

    if (c == '\r')
    {
        unsigned int curr_row = cursor_pos / 80;
        cursor_pos = curr_row * 80;
        fb_move_cursor(cursor_pos);
        return;
    }

    fb_write_cell(cursor_pos, c, FB_WHITE, FB_BLACK);
    cursor_pos++;

    if (cursor_pos >= 80 * 25)
    {
        fb_clear();
        cursor_pos = 0;
    }

    fb_move_cursor(cursor_pos);
}

void fb_write_string(const char* str) {
    while (*str) {
        fb_write_char(*str);
        str++;
    }
}

void fb_backspace(void) {
    if (cursor_pos > 0) {
        cursor_pos--;
        // 用空格覆盖上一个字符
        fb_write_cell(cursor_pos, ' ', FB_WHITE, FB_BLACK);
        fb_move_cursor(cursor_pos);
    }
}

void fb_newline(void) 
{
    // 计算当前行，每行80个字符
    unsigned int current_row = cursor_pos / 80;
    // 移动到下一行开头
    cursor_pos = (current_row + 1) * 80;
    
    // 如果超出屏幕底部，需要滚动屏幕
    if (cursor_pos >= 80 * 25) {
        // 简单的滚动：将所有行上移一行
        for (int i = 0; i < 80 * 24; i++) {
            // 将下一行的内容复制到当前行
            fb[i * 2] = fb[(i + 80) * 2];           // 字符
            fb[i * 2 + 1] = fb[(i + 80) * 2 + 1];   // 颜色属性
        }
        
        // 清空最后一行
        for (int i = 80 * 24; i < 80 * 25; i++) {
            fb_write_cell(i, ' ', FB_WHITE, FB_BLACK);
        }
        
        // 光标移动到最后一行的开头
        cursor_pos = 80 * 24;
    }
    
    fb_move_cursor(cursor_pos);
}
void fb_clear(void) {
    for (int i = 0; i < 80 * 25; i++) {
        fb_write_cell(i, ' ', FB_WHITE, FB_BLACK);
    }
    cursor_pos = 0;
    fb_move_cursor(cursor_pos);
}

void fb_write_hex(u8int value) {
    char hex_chars[] = "0123456789ABCDEF";
    fb_write_char(hex_chars[(value >> 4) & 0x0F]);
    fb_write_char(hex_chars[value & 0x0F]);
}