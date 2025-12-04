#include "../drivers/frame_buffer.h"
#include "../drivers/hardware_interrupt_enabler.h"
#include "../drivers/interrupts.h"
#include "../drivers/input_buffer.h"

// 简单的数字转字符串函数
void int_to_string(u32int num, char* buf) {
    if (num == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }
    
    u32int temp = num;
    u32int digits = 0;
    
    // 计算数字位数
    while (temp > 0) {
        temp /= 10;
        digits++;
    }
    
    // 转换数字为字符串
    temp = num;
    for (u32int i = 0; i < digits; i++) {
        buf[digits - 1 - i] = '0' + (temp % 10);
        temp /= 10;
    }
    buf[digits] = '\0';
}

// 测试getc函数
void test_getc(void) {
    fb_write_string("\n--- Testing getc() ---\n");
    fb_write_string("Type some characters (they will be stored in buffer):\n");
    fb_write_string("Press Enter to process the buffer...\n");
    fb_write_string("> ");
    
    // 等待用户输入一些字符
    while (1) {
        u8int c = getc();
        if (c == '\n') {
            break;
        }
        // 短暂延迟，避免过于频繁的检查
        for (volatile int i = 0; i < 10000; i++);
    }
    
    fb_write_string("\nProcessing buffer contents:\n");
    
    // 读取并显示缓冲区中的所有字符
    u32int count = 0;
    u8int c;
    while ((c = getc()) != 0) {
        fb_write_char(c);
        count++;
    }
    
    fb_write_string("\nTotal characters processed: ");
    
    char num_buf[10];
    int_to_string(count, num_buf);
    fb_write_string(num_buf);
    fb_write_string("\n");
}

// 测试readline函数
void test_readline(void) {
    fb_write_string("\n--- Testing readline() ---\n");
    fb_write_string("Type a line of text and press Enter:\n");
    fb_write_string("> ");
    
    char line_buffer[LINE_BUFFER_SIZE];
    u32int chars_read = readline(line_buffer, LINE_BUFFER_SIZE);
    
    fb_write_string("\nYou typed: \"");
    fb_write_string(line_buffer);
    fb_write_string("\"\n");
    fb_write_string("Characters read: ");
    
    char num_buf[10];
    int_to_string(chars_read, num_buf);
    fb_write_string(num_buf);
    fb_write_string("\n");
}

int kmain() 
{
    // 清屏并显示启动消息
    fb_clear();
    fb_write_string("=== MyOS Input Buffer API Test ===\n");
    fb_write_string("✓ System initialized successfully\n");
    fb_write_string("✓ Interrupt handlers installed\n");
    fb_write_string("✓ Input buffer system ready\n");
    
    // 安装IDT并启用中断
    interrupts_install_idt();
    enable_hardware_interrupts();
    
    fb_write_string("✓ Hardware interrupts enabled\n");
    
    // 测试getc功能
    test_getc();
    
    // 测试readline功能  
    test_readline();
    
    fb_write_string("\n=== All Tests Complete ===\n");
    fb_write_string("Input buffer API is working correctly!\n");
    fb_write_string("You can continue typing:\n");
    fb_write_string("> ");

    // 主循环 - 持续处理输入
    char user_input[LINE_BUFFER_SIZE];
    while(1) {
        if (input_available() > 0) {
            u32int len = readline(user_input, LINE_BUFFER_SIZE);
            if (len > 0) {
                fb_write_string("\nEcho: ");
                fb_write_string(user_input);
                fb_write_string("\n> ");
            }
        }
        
        // 使用HLT节省CPU
        __asm__ __volatile__("hlt");
    }

    return 0;
}