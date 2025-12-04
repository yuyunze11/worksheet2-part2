#include "interrupts.h"
#include "pic.h"
#include "io.h"
#include "frame_buffer.h"
#include "keyboard.h"
#include "input_buffer.h"

#define INTERRUPTS_DESCRIPTOR_COUNT 256
#define INTERRUPTS_KEYBOARD 33

struct IDTDescriptor idt_descriptors[INTERRUPTS_DESCRIPTOR_COUNT];
struct IDT idt;

// 内联实现 load_idt
static void load_idt(u32int idt_address) {
    __asm__ __volatile__("lidt (%0)" : : "r" (idt_address));
}

void interrupts_init_descriptor(s32int index, u32int address)
{
    idt_descriptors[index].offset_high = (address >> 16) & 0xFFFF;
    idt_descriptors[index].offset_low = (address & 0xFFFF);
    idt_descriptors[index].segment_selector = 0x08;
    idt_descriptors[index].reserved = 0x00;
    idt_descriptors[index].type_and_attr = 0x8E;
}

void interrupts_install_idt()
{
    // 初始化输入缓冲区
    input_buffer_init();
    
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

void interrupt_handler(struct cpu_state cpu, u32int interrupt, struct stack_state stack) {
    (void)cpu;
    (void)stack;
    
    switch (interrupt) {
        case INTERRUPTS_KEYBOARD: {
            u8int scan_code;
            u8int ascii;
            
            // 读取扫描码
            scan_code = inb(0x60);
            
            // 只处理按键按下
            if (!(scan_code & 0x80)) {
                ascii = keyboard_scan_code_to_ascii(scan_code);
                
                if (ascii != 0) {
                    // 将字符存入输入缓冲区
                    buffer_put(ascii);
                    
                    // 处理退格键：更新显示
                    if (ascii == '\b') {
                        fb_backspace();
                    }
                    // 处理回车键：更新显示
                    else if (ascii == '\n') {
                        fb_write_string("\n");
                    }
                    // 处理常规字符：更新显示
                    else {
                        fb_write_char(ascii);
                    }
                }
            }
            
            // 确认中断
            pic_acknowledge(interrupt);
            break;
        }

        default:
            // 确认其他可能的中断
            if (interrupt >= PIC_1_OFFSET && interrupt <= PIC_2_END) {
                pic_acknowledge(interrupt);
            }
            break;
    }
}