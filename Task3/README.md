# Low-level OS Modules (Drivers, Interrupts & Terminal)

This directory contains the low-level building blocks of my 32-bit x86 OS:

- VGA text-mode output
- Keyboard input
- PIC + IDT + interrupt handling
- A small input buffer (`readline`)
- A simple text terminal / shell

---

## 1. Basic Types

### `types.h`

Common integer types and framebuffer colour constants used by all drivers. :contentReference[oaicite:0]{index=0}  

```c
typedef unsigned int   u32int;
typedef int            s32int;
typedef unsigned short u16int;
typedef short          s16int;
typedef unsigned char  u8int;
typedef char           s8int;

/* Frame buffer colors */
#define BLACK        0
#define BLUE         1
#define GREEN        2
#define CYAN         3
#define RED          4
#define MAGENTA      5
#define BROWN        6
#define LIGHT_GREY   7
#define DARK_GREY    8
#define LIGHT_BLUE   9
#define LIGHT_GREEN  10
#define LIGHT_CYAN   11
#define LIGHT_RED    12
#define LIGHT_MAGENTA 13
#define YELLOW       14
#define WHITE        15






2. Port I/O
io.h / io.s

Thin wrappers around x86 in / out instructions. Used to talk to the PIC,
keyboard controller, VGA cursor, etc.

unsigned char inb(unsigned short port);
void outb(unsigned short port, unsigned char data);


Example uses:

inb(0x60) – read keyboard scan code

outb(0x3D4, ...) / outb(0x3D5, ...) – move VGA cursor

outb(0x20, 0x20) – send EOI to master PIC

3. Framebuffer (VGA Text Mode)
frame_buffer.h / frame_buffer.c

Writes text directly to VGA text-mode memory at 0xB8000. Keeps track of a
logical cursor and updates the hardware cursor. 

frame_buffer

Core state:

char   *fb        = (char *) 0x000B8000;  /* video memory */
static u16int cursor_pos = 0;             /* current cell index (0..80*25-1) */


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


Example (simplified):

void fb_write_char(char c) {
    if (c == '\n') {
        fb_newline();
        return;
    }
    fb_write_cell(cursor_pos, c, WHITE, BLACK);
    cursor_pos++;
    fb_move_cursor(cursor_pos);
}

4. PIC – Programmable Interrupt Controller
pic.h / pic.c

Configures and acknowledges the 8259 PIC.

Key definitions:

#define PIC_1        0x20
#define PIC_2        0xA0
#define PIC_1_COMMAND PIC_1
#define PIC_1_DATA   (PIC_1+1)
#define PIC_2_COMMAND PIC_2
#define PIC_2_DATA   (PIC_2+1)

#define PIC_1_OFFSET 0x20
#define PIC_2_OFFSET 0x28
#define PIC_2_END    (PIC_2_OFFSET + 7)


API:

void pic_remap(s32int offset1, s32int offset2);
void pic_acknowledge(u32int interrupt);


pic_remap() moves IRQ0–15 to new interrupt numbers (e.g. 0x20–0x2F).

pic_acknowledge() sends an EOI (0x20) to the correct PIC after an interrupt.

5. Keyboard Driver
keyboard.h / keyboard.c

Reads scan codes from the keyboard controller (0x60) and converts them to
ASCII characters (US QWERTY).

API:

u8int keyboard_read_scan_code(void);
u8int keyboard_scan_code_to_ascii(u8int scan_code);


Implementation (excerpt):

u8int keyboard_read_scan_code(void) {
    return inb(0x60);
}

u8int keyboard_scan_code_to_ascii(u8int scan_code) {
    if (scan_code & 0x80) {
        return 0;              /* ignore key release */
    }

    switch (scan_code) {
        case 0x02: return '1';
        case 0x10: return 'q';
        case 0x1E: return 'a';
        /* ... more keys ... */
        case 0x39: return ' ';   /* space */
        case 0x1C: return '\n';  /* Enter */
        case 0x0E: return '\b';  /* Backspace */
        default:   return 0;
    }
}

6. Interrupt Descriptors & C Handler
interrupts.h / interrupts.c

Sets up the IDT and implements the common C-level interrupt handler.

Important pieces:

struct IDTDescriptor idt_descriptors[256];

struct IDT idt;

interrupts_init_descriptor(index, address) – fills one IDT entry.

interrupts_install_idt() – installs the IDT, remaps the PIC, unmasks only
IRQ1 (keyboard).

Keyboard interrupt handling (simplified):

void interrupt_handler(struct cpu_state cpu, u32int interrupt,
                       struct stack_state stack)
{
    (void)cpu; (void)stack;

    switch (interrupt) {
        case 33: {                 /* IRQ1 -> int 33 */
            u8int scan = inb(0x60);
            u8int ch   = keyboard_scan_code_to_ascii(scan);

            if (ch != 0) {
                /* either print directly or push into input buffer */
                // fb_write_char(ch);
                // buffer_put(ch);
            }

            pic_acknowledge(interrupt);
            break;
        }
        default:
            /* acknowledge other PIC interrupts as well */
            pic_acknowledge(interrupt);
            break;
    }
}

7. Assembly Interrupt Stubs
interrupt_asm.s

Assembly glue that the CPU enters first, before C code. 

interrupt_asm

Typical pattern:

global interrupt_handler_33

interrupt_handler_33:
    push dword 0       ; fake error code
    push dword 33      ; interrupt number
    jmp  common_interrupt_handler

common_interrupt_handler:
    pusha
    ; ... save segments if needed ...
    push esp
    call interrupt_handler   ; C function
    add  esp, 4
    ; ... restore segments ...
    popa
    add  esp, 8              ; drop error code + int number
    iretd


The C function interrupt_handler(...) never sees the raw CPU entry; it always
comes through this stub.

8. Enabling / Disabling Hardware Interrupts
hardware_interrupt_enabler.s

Very small assembly helpers around sti / cli:

void enable_hardware_interrupts(void);  /* sti */
void disable_hardware_interrupts(void); /* cli */


Typically called from kmain after IDT + PIC setup is complete.

9. Input Buffer & readline
input_buffer.h / input_buffer.c

Implements a ring buffer for keyboard characters and a simple readline
function (character-level input from interrupts, line-level API for higher
layers). 

input_buffer

Key functions:

void  input_buffer_init(void);
void  buffer_put(u8int c);
u8int getc(void);
u32int input_available(void);
u32int readline(char *buf, u32int max_len);


buffer_put() is called from the keyboard interrupt handler.

readline():

repeatedly calls getc(),

uses hlt when there is no input,

echoes characters with fb_write_char,

handles backspace with fb_backspace,

stops at '\n' and returns the line.

10. Terminal (Tiny Shell)
terminal.h / terminal.c

A small text terminal built on top of the framebuffer and input buffer.

Command structure:

struct command {
    const char* name;
    void (*function)(char* args);
    const char* description;
};


Command table:

static struct command commands[] = {
    {"echo",     cmd_echo,     "Display the provided text"},
    {"clear",    cmd_clear,    "Clear the screen"},
    {"help",     cmd_help,     "Display available commands"},
    {"version",  cmd_version,  "Display OS version"},
    {"shutdown", cmd_shutdown, "Prepare system for shutdown"},
    {0, 0, 0}
};


Main functions:

terminal_init() – clear screen, print a welcome banner.

terminal_prompt() – print prompt like myos> .

terminal_run() – main loop:

print prompt,

call readline(...) to read one full line,

call terminal_execute(input).

terminal_execute() – parses the command name + arguments and calls the
matching cmd_* function, or prints “Unknown command” if not found.

Example command (echo):

void cmd_echo(char* args) {
    if (*args == '\0') {
        fb_write_string("Usage: echo <text>\n");
        return;
    }
    fb_write_string(args);
    fb_write_string("\n");
}

11. Overall Flow (Keyboard → Terminal → Screen)

You press a key → keyboard controller generates a scan code.

PIC raises IRQ1 → mapped to interrupt 33.

CPU uses the IDT → jumps into interrupt_handler_33 (assembly).

Assembly stub saves registers → calls C interrupt_handler.

C handler reads scan code from port 0x60 → converts to ASCII via keyboard
driver → pushes it into the input buffer.

The terminal calls readline() to get full lines, parses commands, and uses
the framebuffer driver to print output back to the screen.

This connects hardware interrupts all the way up to a tiny shell.