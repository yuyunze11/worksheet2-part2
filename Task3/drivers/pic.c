#include "pic.h"
#include "io.h"

void pic_remap(s32int offset1, s32int offset2)
{
    u8int a1, a2;

    a1 = inb(PIC_1_DATA);  // 保存掩码
    a2 = inb(PIC_2_DATA);

    // 初始化主PIC
    outb(PIC_1_COMMAND, PIC_ICW1_INIT | PIC_ICW1_ICW4);
    outb(PIC_2_COMMAND, PIC_ICW1_INIT | PIC_ICW1_ICW4);
    
    // 设置偏移量
    outb(PIC_1_DATA, offset1);
    outb(PIC_2_DATA, offset2);
    
    // 告诉主PIC有从PIC在IRQ2
    outb(PIC_1_DATA, 4);
    // 告诉从PIC它的级联标识
    outb(PIC_2_DATA, 2);
    
    // 8086模式
    outb(PIC_1_DATA, PIC_ICW4_8086);
    outb(PIC_2_DATA, PIC_ICW4_8086);
    
    // 恢复保存的掩码
    outb(PIC_1_DATA, a1);
    outb(PIC_2_DATA, a2);
}

void pic_acknowledge(u32int interrupt)
{
    if (interrupt >= PIC_1_OFFSET && interrupt <= PIC_2_END) {
        if (interrupt < PIC_2_OFFSET) {
            outb(PIC_1_COMMAND_PORT, PIC_ACKNOWLEDGE);
        } else {
            outb(PIC_2_COMMAND_PORT, PIC_ACKNOWLEDGE);
        }
    }
}