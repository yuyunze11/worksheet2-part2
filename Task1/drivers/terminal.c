#include "terminal.h"
#include "frame_buffer.h"
#include "input_buffer.h"
#include "io.h"

// 命令表
static struct command commands[] = {
    {"echo", cmd_echo, "Display the provided text"},
    {"clear", cmd_clear, "Clear the screen"},
    {"help", cmd_help, "Display available commands"},
    {"version", cmd_version, "Display OS version"},
    {"shutdown", cmd_shutdown, "Prepare system for shutdown"},
    {0, 0, 0}  // 结束标记
};

// 系统信息
static const char* OS_NAME = "MyOS";
static const char* OS_VERSION = "1.0.0";

// 初始化终端
void terminal_init(void) {
    fb_clear();
    fb_write_string("=== ");
    fb_write_string(OS_NAME);
    fb_write_string(" Terminal ===\n");
    fb_write_string("Type 'help' for available commands\n\n");
}

// 显示提示符
void terminal_prompt(void) {
    fb_write_string("myos> ");
}

// 运行终端主循环
void terminal_run(void) {
    char input[LINE_BUFFER_SIZE];
    
    while (1) {
        terminal_prompt();
        
        // 读取用户输入
        u32int len = readline(input, LINE_BUFFER_SIZE);
        
        if (len > 0) {
            terminal_execute(input);
        }
        
        fb_write_string("\n");
    }
}

// 解析和执行命令
void terminal_execute(char* input) {
    // 跳过前导空格
    while (*input == ' ') input++;
    
    // 检查空输入
    if (*input == '\0') {
        return;
    }
    
    // 查找命令结束位置（空格或字符串结束）
    char* command_end = input;
    while (*command_end != ' ' && *command_end != '\0') {
        command_end++;
    }
    
    // 提取命令名
    char command_name[32];
    u32int command_len = command_end - input;
    if (command_len >= sizeof(command_name)) {
        command_len = sizeof(command_name) - 1;
    }
    
    for (u32int i = 0; i < command_len; i++) {
        command_name[i] = input[i];
    }
    command_name[command_len] = '\0';
    
    // 提取参数（跳过命令后的空格）
    char* args = command_end;
    while (*args == ' ') args++;
    
    // 在命令表中查找命令
    struct command* cmd = commands;
    while (cmd->name != 0) {
        // 简单的字符串比较
        u32int i = 0;
        u8int match = 1;
        
        while (command_name[i] != '\0' && cmd->name[i] != '\0') {
            if (command_name[i] != cmd->name[i]) {
                match = 0;
                break;
            }
            i++;
        }
        
        // 确保两个字符串同时结束
        if (match && command_name[i] == '\0' && cmd->name[i] == '\0') {
            // 找到命令，执行
            cmd->function(args);
            return;
        }
        
        cmd++;
    }
    
    // 命令未找到
    fb_write_string("Unknown command: '");
    fb_write_string(command_name);
    fb_write_string("'\n");
    fb_write_string("Type 'help' for available commands\n");
}

// echo命令：显示提供的文本
void cmd_echo(char* args) {
    if (*args == '\0') {
        fb_write_string("Usage: echo <text>\n");
        return;
    }
    
    fb_write_string(args);
    fb_write_string("\n");
}

// clear命令：清屏
void cmd_clear(char* args) {
    (void)args; // 未使用参数
    fb_clear();
}

// help命令：显示可用命令
void cmd_help(char* args) {
    (void)args; // 未使用参数
    
    fb_write_string("Available commands:\n");  // 确保换行
    fb_write_string("===================\n");
    
    struct command* cmd = commands;
    while (cmd->name != 0) {
        fb_write_string("  ");
        fb_write_string(cmd->name);
        
        // 计算命令名长度用于对齐
        u32int name_len = 0;
        const char* name_ptr = cmd->name;
        while (*name_ptr != '\0') {
            name_len++;
            name_ptr++;
        }
        
        // 使用空格而不是制表符进行对齐
        // 根据名称长度添加适当数量的空格
        if (name_len < 8) {
            fb_write_string("        ");  // 8个空格
        } else {
            fb_write_string("    ");      // 4个空格
        }
        
        fb_write_string("- ");  // 使用连字符作为分隔符
        fb_write_string(cmd->description);
        fb_write_string("\n");  // 确保每个命令换行
        
        cmd++;
    }
}

// version命令：显示操作系统版本
void cmd_version(char* args) {
    (void)args; // 未使用参数
    
    fb_write_string(OS_NAME);
    fb_write_string(" version ");
    fb_write_string(OS_VERSION);
    fb_write_string("\n");
    fb_write_string("Simple operating system for educational purposes\n");
}

// shutdown命令：准备系统关机
void cmd_shutdown(char* args) {
    (void)args; // 未使用参数
    
    fb_write_string("System is shutting down...\n");
    fb_write_string("Thank you for using ");
    fb_write_string(OS_NAME);
    fb_write_string("!\n");
    
    // 在实际操作系统中，这里会执行真正的关机程序
    // 现在我们只是显示消息并停止接受新命令
    while (1) {
        __asm__ __volatile__("hlt");
    }
}