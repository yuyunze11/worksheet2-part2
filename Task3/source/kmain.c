#include "../drivers/frame_buffer.h"
#include "../drivers/hardware_interrupt_enabler.h"
#include "../drivers/interrupts.h"
#include "../drivers/input_buffer.h"
#include "../drivers/terminal.h"

int kmain() 
{
    // 清屏并显示启动消息
    fb_clear();
    fb_write_string("=== MyOS Booting ===\n");
    fb_write_string("Initializing system components...\n");
    
    // 安装IDT并启用中断
    interrupts_install_idt();
    enable_hardware_interrupts();
    
    fb_write_string("✓ Interrupt system ready\n");
    fb_write_string("✓ Input buffer cleinitialized\n");
    fb_write_string("✓ Terminal system ready\n");
    
    // 初始化并运行终端
    terminal_init();
    terminal_run();

    // 正常情况下不会到达这里
    return 0;
}