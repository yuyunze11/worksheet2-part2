#include "input_buffer.h"
#include "io.h"
#include "frame_buffer.h"

// 循环缓冲区结构
static struct {
    u8int buffer[INPUT_BUFFER_SIZE];
    u32int read_index;
    u32int write_index;
    u32int count;  // 当前缓冲区中的字符数
} input_buffer;

// 初始化输入缓冲区
void input_buffer_init(void) {
    input_buffer.read_index = 0;
    input_buffer.write_index = 0;
    input_buffer.count = 0;
}

// 向缓冲区添加一个字符
void buffer_put(u8int c) {
    if (input_buffer.count >= INPUT_BUFFER_SIZE) {
        // 缓冲区已满，丢弃最老的字符
        input_buffer.read_index = (input_buffer.read_index + 1) % INPUT_BUFFER_SIZE;
        input_buffer.count--;
    }
    
    input_buffer.buffer[input_buffer.write_index] = c;
    input_buffer.write_index = (input_buffer.write_index + 1) % INPUT_BUFFER_SIZE;
    input_buffer.count++;
}

// 从缓冲区获取一个字符（非阻塞）
u8int getc(void) {
    if (input_buffer.count == 0) {
        return 0;  // 缓冲区为空
    }
    
    u8int c = input_buffer.buffer[input_buffer.read_index];
    input_buffer.read_index = (input_buffer.read_index + 1) % INPUT_BUFFER_SIZE;
    input_buffer.count--;
    
    return c;
}

// 检查缓冲区中是否有数据
u32int input_available(void) {
    return input_buffer.count;
}

// 读取一行输入
u32int readline(char* buf, u32int max_len) {
    u32int index = 0;
    u8int c;
    
    if (max_len == 0) {
        return 0;  // 无效的缓冲区大小
    }
    
    // 确保缓冲区以空字符结尾
    buf[0] = '\0';
    
    while (index < max_len - 1) {  // 为终止符留出空间
        c = getc();
        
        if (c == 0) {
            // 没有可用字符，等待一下
            // 在实际操作系统中，这里应该让出CPU，但我们现在简单循环
            continue;
        }
        
        // 处理回车键（换行）
        if (c == '\n') {
            buf[index] = '\0';  // 字符串终止符
            return index;
        }
        
        // 处理退格键
        if (c == '\b') {
            if (index > 0) {
                index--;
                // 在屏幕上显示退格效果（可选）
                fb_backspace();
            }
            continue;
        }
        
        // 常规字符
        if (c >= 32 && c <= 126) {  // 可打印ASCII字符
            buf[index++] = c;
            // 在屏幕上回显字符（可选）
            fb_write_char(c);
        }
    }
    
    // 缓冲区已满，添加终止符
    buf[index] = '\0';
    return index;
}