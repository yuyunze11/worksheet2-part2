#include "interrupts.h"
#include "pic.h"
#include "io.h"
#include "frame_buffer.h"
#include "keyboard.h"

#define INTERRUPTS_DESCRIPTOR_COUNT 256
#define INTERRUPTS_KEYBOARD 33

struct IDTDescriptor idt_descriptors[INTERRUPTS_DESCRIPTOR_COUNT];
struct IDT idt;

void interrupts_init_descriptor(s32int index, u32int address)
{
    idt_descriptors[index].offset_high = (address >> 16) & 0xFFFF;
    idt_descriptors[index].offset_low = (address & 0xFFFF);
    idt_descriptors[index].segment_selector = 0x08;
    idt_descriptors[index].reserved = 0x00;
    idt_descriptors[index].type_and_attr = 0x8E; // Present, 32-bit interrupt gate
}

void interrupts_install_idt()
{
    // 初始化所有IDT条目为安全值
    for(int i = 0; i < INTERRUPTS_DESCRIPTOR_COUNT; i++) {
        // 设置一个默认的中断处理程序（指向一个安全的空函数）
        // 暂时不设置，只设置键盘中断
    }
    
    // 只设置键盘中断
    interrupts_init_descriptor(INTERRUPTS_KEYBOARD, (u32int) interrupt_handler_33);

    idt.address = (s32int) &idt_descriptors;
    idt.size = sizeof(struct IDTDescriptor) * INTERRUPTS_DESCRIPTOR_COUNT - 1;
    load_idt((s32int) &idt);

    // PIC重新映射
    pic_remap(PIC_1_OFFSET, PIC_2_OFFSET);

    // 只启用键盘中断
    outb(0x21, 0xFD); // 11111101 - 只启用IRQ1（键盘）
    outb(0xA1, 0xFF); // 11111111 - 禁用所有从PIC中断
}

// 最简单的键盘中断处理程序
void interrupt_handler(struct cpu_state cpu, u32int interrupt, struct stack_state stack) {
    // 使用(void)来忽略未使用的参数，消除警告
    (void)cpu;
    (void)stack;
    
    // 在屏幕右上角显示中断号（调试用）
    unsigned int debug_pos = 79; // 右上角
    char int_char = '0' + (interrupt % 10);
    if (interrupt >= 10) int_char = 'A' + (interrupt - 10);
    fb_write_cell(debug_pos, int_char, FB_LIGHT_RED, FB_BLACK);
    
    switch (interrupt) {
        case INTERRUPTS_KEYBOARD: {
            // 在case语句中使用花括号创建作用域，然后声明变量
            u8int scan_code;
            u8int ascii;
            
            // 读取扫描码
            scan_code = inb(0x60);
            
            // 在屏幕另一个位置显示扫描码（调试）
            fb_write_cell(78, 'K', FB_LIGHT_GREEN, FB_BLACK); // 显示K表示键盘中断
            
            // 只处理按键按下
            if (!(scan_code & 0x80)) {
                ascii = keyboard_scan_code_to_ascii(scan_code);
                
                if (ascii != 0) {
                    // 显示字符
                    fb_write_char(ascii);
                }
            }
            
            // 必须确认中断！
            pic_acknowledge(interrupt);
            break;
        }

        default:
            // 其他中断也要确认
            if (interrupt >= PIC_1_OFFSET && interrupt <= PIC_2_END) {
                pic_acknowledge(interrupt);
            }
            break;
    }
}