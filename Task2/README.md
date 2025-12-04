# Low-level OS Code Overview

This code implements the low-level building blocks of a tiny 32-bit x86 OS:

- VGA text-mode output (framebuffer)
- Keyboard input
- PIC + IDT + interrupt handlers
- A small input buffer (`readline`-style)
- Helpers to enable/disable hardware interrupts

---

## 1. Basic Types

### `types.h`

Defines fixed-width integer types and colour constants used across the kernel:

```c
typedef unsigned int   u32int;
typedef int            s32int;
typedef unsigned short u16int;
typedef short          s16int;
typedef unsigned char  u8int;
typedef char           s8int;

/* example colours */
#define BLACK   0
#define BLUE    1
#define GREEN   2
#define WHITE   15



2. Port I/O
io.h / io.s

Thin wrappers around the x86 in / out instructions:

/* io.h */
unsigned char inb(unsigned short port);
void outb(unsigned short port, unsigned char data);


Used by:

framebuffer (move hardware cursor),

PIC (configure and acknowledge interrupts),

keyboard (read scan codes from port 0x60).

3. Framebuffer (Screen Output)
frame_buffer.h / frame_buffer.c

Simple VGA text-mode driver at physical address 0xB8000.

Core state:

char  *fb         = (char *) 0x000B8000;  /* video memory */
static u16int cursor_pos = 0;             /* current cursor cell index */


Main functions:

void fb_write_cell(unsigned int i, char c,
                   unsigned char fg, unsigned char bg);
void fb_move_cursor(u16int pos);
void fb_write_char(char c);
void fb_write_string(const char *str);
void fb_backspace(void);
void fb_newline(void);
void fb_clear(void);
void fb_write_hex(u8int value);


Example (simplified) fb_write_char:

void fb_write_char(char c) {
    if (c == '\n') {
        fb_newline();
        return;
    }

    fb_write_cell(cursor_pos, c, WHITE, BLACK);
    cursor_pos++;
    fb_move_cursor(cursor_pos);
}


All visible text (boot messages, echoing typed keys, etc.) goes through this module.

4. PIC – Programmable Interrupt Controller
pic.h / pic.c

Configures the 8259 PIC and acknowledges hardware interrupts.

Key functions:

void pic_remap(u8int offset1, u8int offset2);
void pic_acknowledge(u32int interrupt);


Typical usage:

pic_remap(0x20, 0x28); remaps IRQ0–15 to interrupts 0x20–0x2F.

pic_acknowledge(interrupt); sends EOI after handling an interrupt.

5. Keyboard Driver
keyboard.h / keyboard.c

Converts keyboard scan codes to ASCII characters.

API:

u8int keyboard_read_scan_code(void);
u8int keyboard_scan_code_to_ascii(u8int scan_code);


Simplified mapping example:

u8int keyboard_scan_code_to_ascii(u8int scan_code) {
    /* bit 7 set => key release; ignore */
    if (scan_code & 0x80) {
        return 0;
    }

    switch (scan_code) {
        case 0x1E: return 'a';
        case 0x30: return 'b';
        /* ... other letters ... */
        case 0x2C: return 'z';
        case 0x39: return ' ';   /* space */
        case 0x1C: return '\n';  /* Enter */
        case 0x0E: return '\b';  /* Backspace */
        default:   return 0;
    }
}


The interrupt handler uses this to turn raw scan codes into printable characters.

6. Interrupt Descriptors & C-level Handler
interrupts.h / interrupts.c

Sets up the IDT and implements the common C interrupt handler.

Key structures (simplified):

struct IDTDescriptor {
    u16int offset_low;
    u16int segment_selector;
    u8int  reserved;
    u8int  type_and_attr;
    u16int offset_high;
};

struct IDT {
    u16int size;
    u32int address;
};


Setting one IDT entry:

void interrupts_init_descriptor(s32int index, u32int address) {
    idt_descriptors[index].offset_low  = address & 0xFFFF;
    idt_descriptors[index].offset_high = (address >> 16) & 0xFFFF;
    idt_descriptors[index].segment_selector = 0x08;   /* code segment */
    idt_descriptors[index].type_and_attr    = 0x8E;   /* present, int gate */
}


Installing IDT and enabling only keyboard IRQ:

void interrupts_install_idt(void) {
    interrupts_init_descriptor(33, (u32int) interrupt_handler_33);

    idt.address = (u32int) &idt_descriptors;
    idt.size    = sizeof(idt_descriptors) - 1;
    load_idt((u32int) &idt);

    pic_remap(PIC_1_OFFSET, PIC_2_OFFSET);

    /* unmask only IRQ1 (keyboard) on master PIC */
    outb(0x21, 0xFD);  /* 11111101b */
    outb(0xA1, 0xFF);  /* mask all on slave PIC */
}


Keyboard interrupt handling (simplified):

void interrupt_handler(struct cpu_state cpu, u32int interrupt,
                       struct stack_state stack)
{
    (void)cpu; (void)stack;

    switch (interrupt) {
        case 33: {  /* keyboard IRQ1 */
            u8int scan = inb(0x60);
            u8int ch   = keyboard_scan_code_to_ascii(scan);

            if (ch != 0) {
                /* Option A: print directly */
                fb_write_char(ch);

                /* Option B (extended): buffer_put(ch); */
            }

            pic_acknowledge(interrupt);
            break;
        }

        default:
            pic_acknowledge(interrupt);
            break;
    }
}

7. Assembly Interrupt Stubs
interrupt_asm.s / interrupt_handlers.s

Assembly glue between the CPU’s int mechanism and the C interrupt_handler.

Typical pattern:

global interrupt_handler_33

interrupt_handler_33:
    push    dword 0      ; fake error code
    push    dword 33     ; interrupt number
    jmp     common_interrupt_handler

common_interrupt_handler:
    pusha
    push    ds
    push    es
    ; call C handler
    push    esp
    call    interrupt_handler
    add     esp, 4
    pop     es
    pop     ds
    popa
    add     esp, 8       ; remove error code + int number
    iretd


The CPU jumps into interrupt_handler_33; the stub saves registers, calls the C
handler, restores state, and finishes with iret.

8. Enabling / Disabling Hardware Interrupts
hardware_interrupt_enabler.h / hardware_interrupt_enabler.s

Small helpers around sti / cli:

void enable_hardware_interrupts(void);  /* uses sti */
void disable_hardware_interrupts(void); /* uses cli */


Used in kmain after IDT and PIC are fully configured.

9. Input Buffer & readline
input_buffer.h / input_buffer.c

Implements a ring buffer for keyboard characters and a helper to read a full line.

API:

void  input_buffer_init(void);
void  buffer_put(u8int c);
u8int getc(void);
u32int input_available(void);
u32int readline(char *buf, u32int max_len);


buffer_put (called from the interrupt handler):

void buffer_put(u8int c) {
    if (input_buffer.count == INPUT_BUFFER_SIZE) {
        /* drop oldest char when full */
        input_buffer.read_index =
            (input_buffer.read_index + 1) % INPUT_BUFFER_SIZE;
        input_buffer.count--;
    }
    input_buffer.buffer[input_buffer.write_index] = c;
    input_buffer.write_index =
        (input_buffer.write_index + 1) % INPUT_BUFFER_SIZE;
    input_buffer.count++;
}


readline (simplified):

u32int readline(char *buf, u32int max_len) {
    u32int index = 0;

    while (index < max_len - 1) {
        u8int c = getc();

        if (c == 0) {
            __asm__ __volatile__("hlt");  /* sleep until next interrupt */
            continue;
        }

        if (c == '\n') {
            buf[index] = '\0';
            fb_write_char('\n');
            return index;
        }

        if (c == '\b') {
            if (index > 0) {
                index--;
                fb_backspace();
            }
            continue;
        }

        buf[index++] = c;
        fb_write_char(c);
    }

    buf[index] = '\0';
    return index;
}


This lets higher-level code read line-based input without dealing with raw
interrupts directly.

One-line Summary

Port I/O (io) + drivers (frame_buffer, keyboard, pic) + IDT/ISR
(interrupts, interrupt_asm) + global enable (hardware_interrupt_enabler)

buffering (input_buffer) together form a minimal interrupt-driven input
and output system for the OS.