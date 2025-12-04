#include "interrupts.h"
#include "pic.h"
#include "io.h"
#include "frame_buffer.h"
#include "keyboard.h"

#define INTERRUPTS_DESCRIPTOR_COUNT 256
#define INTERRUPTS_KEYBOARD         33

// 这里只需要声明在汇编里实现的中断 stub 即可
extern void interrupt_handler_33(void);

struct IDTDescriptor idt_descriptors[INTERRUPTS_DESCRIPTOR_COUNT];
struct IDT idt;

void interrupts_init_descriptor(s32int index, u32int address)
{
    idt_descriptors[index].offset_high = (address >> 16) & 0xFFFF;
    idt_descriptors[index].offset_low  = (address & 0xFFFF);
    idt_descriptors[index].segment_selector = 0x08;
    idt_descriptors[index].reserved         = 0x00;
    idt_descriptors[index].type_and_attr    = 0x8E; // Present, 32-bit interrupt gate
}

void interrupts_install_idt(void)
{
    // 这里只设置键盘中断
    interrupts_init_descriptor(INTERRUPTS_KEYBOARD, (u32int)interrupt_handler_33);

    idt.address = (s32int)&idt_descriptors;
    idt.size    = sizeof(struct IDTDescriptor) * INTERRUPTS_DESCRIPTOR_COUNT - 1;

    // 这里直接用头文件里的声明：void load_idt(u32int idt_address);
    load_idt((u32int)&idt);

    // PIC 重新映射
    pic_remap(PIC_1_OFFSET, PIC_2_OFFSET);

    // 只启用键盘中断
    outb(0x21, 0xFD); // 11111101 - 只启用 IRQ1（键盘）
    outb(0xA1, 0xFF); // 11111111 - 禁用所有从 PIC 中断
}

// 最简单的键盘中断处理程序
void interrupt_handler(struct cpu_state cpu, u32int interrupt, struct stack_state stack)
{
    (void)cpu;
    (void)stack;

    // 在屏幕右上角显示中断号（调试用）
    unsigned int debug_pos = 79; // 右上角
    char int_char = '0' + (interrupt % 10);
    if (interrupt >= 10) int_char = 'A' + (interrupt - 10);
    fb_write_cell(debug_pos, int_char, FB_LIGHT_RED, FB_BLACK);

    switch (interrupt) {
        case INTERRUPTS_KEYBOARD: {
            u8int scan_code;
            u8int ascii;

            // 读取扫描码
            scan_code = inb(0x60);

            // 显示 K 表示键盘中断
            fb_write_cell(78, 'K', FB_LIGHT_GREEN, FB_BLACK);

            // 只处理按键按下
            if (!(scan_code & 0x80)) {
                ascii = keyboard_scan_code_to_ascii(scan_code);

                if (ascii != 0) {
                    fb_write_char(ascii);
                    // 这里以后可以加：把 ascii 放进输入缓冲区，配合 readline 使用
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
