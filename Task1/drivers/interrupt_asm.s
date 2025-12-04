[bits 32]

; C 里实现的通用中断处理函数
extern interrupt_handler

;-------------------------------------------------------------------------------
; void load_idt(u32int idt_address);
; 在 C 里声明于 interrupts.h，由 interrupts.c 调用
; 这里只做一个简单的 lidt 包装
;-------------------------------------------------------------------------------
global load_idt
load_idt:
    ; C 调用约定：第一个参数在 [esp+4]
    mov eax, [esp + 4]   ; eax = &idt  (struct IDT *)
    lidt [eax]           ; 加载 IDT
    ret

;-------------------------------------------------------------------------------
; 通用中断 handler stub 宏
;-------------------------------------------------------------------------------

%macro no_error_code_interrupt_handler 1
global interrupt_handler_%1
interrupt_handler_%1:
    push dword 0      ; 伪造一个 error code = 0
    push dword %1     ; 压入中断号
    jmp common_interrupt_handler
%endmacro

%macro error_code_interrupt_handler 1
global interrupt_handler_%1
interrupt_handler_%1:
    ; 注意：这里假设 CPU 已经自动压入了 error code
    ; 这里只需要压入中断号即可
    push dword %1
    jmp common_interrupt_handler
%endmacro

;-------------------------------------------------------------------------------
; 通用中断处理入口
; 这里的栈布局要和 C 里的 struct cpu_state / struct stack_state 对得上
;-------------------------------------------------------------------------------
common_interrupt_handler:
    ; 保存通用寄存器
    push eax
    push ebx
    push ecx
    push edx
    push ebp
    push esi
    push edi

    ; 调用 C 的通用中断处理函数：
    ; void interrupt_handler(struct cpu_state cpu,
    ;                        u32int interrupt,
    ;                        struct stack_state stack);
    ;
    ; 这里不额外构造参数，直接利用当前栈布局，
    ; 让 C 按照约定从栈上读取这三个参数。
    call interrupt_handler

    ; 恢复寄存器
    pop edi
    pop esi
    pop ebp
    pop edx
    pop ecx
    pop ebx
    pop eax

    ; 把之前压的 error_code 和 interrupt number 弹掉
    add esp, 8

    ; 从中断返回
    iret

;-------------------------------------------------------------------------------
; 具体中断向量的 stub
; 这里只生成 33 号中断（键盘）
;-------------------------------------------------------------------------------
no_error_code_interrupt_handler 33    ; interrupt_handler_33
