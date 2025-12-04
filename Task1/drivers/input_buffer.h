#ifndef INCLUDE_INPUT_BUFFER_H
#define INCLUDE_INPUT_BUFFER_H

#include "types.h"

#define INPUT_BUFFER_SIZE 256  // 缓冲区大小
#define LINE_BUFFER_SIZE 128   // 行缓冲区大小

// 初始化输入缓冲区系统
void input_buffer_init(void);

// 向缓冲区添加字符（供中断处理程序使用）
void buffer_put(u8int c);

// 从缓冲区获取一个字符（非阻塞）
// 返回：获取的字符，如果缓冲区为空则返回0
u8int getc(void);

// 读取一行输入
// buf: 存储输入的缓冲区
// max_len: 缓冲区最大长度
// 返回：实际读取的字符数
u32int readline(char* buf, u32int max_len);

// 检查缓冲区中是否有数据
u32int input_available(void);

#endif /* INCLUDE_INPUT_BUFFER_H */