#include "pic.h"
#include "io.h"

void pic_remap(s32int offset1, s32int offset2)
{
    u8int a1, a2;

    a1 = inb(PIC_1_DATA);                        // save masks
    a2 = inb(PIC_2_DATA);

    outb(PIC_1_COMMAND, PIC_ICW1_INIT | PIC_ICW1_ICW4);  // starts the initialization sequence (in cascade mode)
    outb(PIC_2_COMMAND, PIC_ICW1_INIT | PIC_ICW1_ICW4);
    outb(PIC_1_DATA, offset1);                 // ICW2: Master PIC vector offset
    outb(PIC_2_DATA, offset2);                 // ICW2: Slave PIC vector offset
    outb(PIC_1_DATA, 4);                       // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
    outb(PIC_2_DATA, 2);                       // ICW3: tell Slave PIC its cascade identity (0000 0010)

    outb(PIC_1_DATA, PIC_ICW4_8086);
    outb(PIC_2_DATA, PIC_ICW4_8086);

    outb(PIC_1_DATA, a1);   // restore saved masks.
    outb(PIC_2_DATA, a2);
}

void pic_acknowledge(u32int interrupt)
{
    if (interrupt < PIC_1_OFFSET || interrupt > PIC_2_END) 
    {
        return;
    }

    if (interrupt < PIC_2_OFFSET) {
        outb(PIC_1_COMMAND_PORT, PIC_ACKNOWLEDGE);
    } else {
        outb(PIC_2_COMMAND_PORT, PIC_ACKNOWLEDGE);
    }
}