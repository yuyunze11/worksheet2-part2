#ifndef INCLUDE_INTERRUPTS
#define INCLUDE_INTERRUPTS

#include "types.h"

struct IDT
{
    u16int size;
    u32int address;
} __attribute__((packed));

struct IDTDescriptor {
    u16int offset_low;
    u16int segment_selector;
    u8int reserved;
    u8int type_and_attr;
    u16int offset_high;
} __attribute__((packed));

struct cpu_state {
    u32int eax;
    u32int ebx;
    u32int ecx;
    u32int edx;
    u32int ebp;
    u32int esi;
    u32int edi;
} __attribute__((packed));

struct stack_state {
    u32int error_code;
    u32int eip;
    u32int cs;
    u32int eflags;
} __attribute__((packed));

void interrupt_handler(struct cpu_state cpu, u32int interrupt, struct stack_state stack);
void interrupts_install_idt();

// Wrappers around ASM.
void interrupt_handler_33();

#endif /* INCLUDE_INTERRUPTS */