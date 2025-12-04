#ifndef INCLUDE_TERMINAL_H
#define INCLUDE_TERMINAL_H

#include "types.h"

// 命令结构
struct command {
    const char* name;
    void (*function)(char* args);
    const char* description;
};

// 终端初始化
void terminal_init(void);

// 运行终端主循环
void terminal_run(void);

// 显示提示符
void terminal_prompt(void);

// 解析和执行命令
void terminal_execute(char* input);

// 命令函数声明
void cmd_echo(char* args);
void cmd_clear(char* args);
void cmd_help(char* args);
void cmd_version(char* args);
void cmd_shutdown(char* args);

#endif /* INCLUDE_TERMINAL_H */