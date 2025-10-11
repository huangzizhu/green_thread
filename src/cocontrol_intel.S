# Intel 语法版本
.intel_syntax noprefix
.text
.global make_fcontext
.global jump_fcontext
.type make_fcontext, @function
.type jump_fcontext, @function

make_fcontext:
    # 从栈顶预留上下文保存区
    lea     rax, [rdi - 0x40]       # rax = stack_top - 64

    # 计算对齐的栈指针
    lea     rcx, [rdi - 8]          # 预留空间
    and     rcx, -16                # 16字节对齐

    # 初始化上下文
    mov     QWORD PTR [rax + 0x00], rcx    # ctx->rsp
    xor     rcx, rcx
    mov     QWORD PTR [rax + 0x08], rcx    # ctx->rbp = 0
    mov     QWORD PTR [rax + 0x10], rcx    # ctx->rbx = 0
    mov     QWORD PTR [rax + 0x18], rcx    # ctx->r12 = 0
    mov     QWORD PTR [rax + 0x20], rcx    # ctx->r13 = 0
    mov     QWORD PTR [rax + 0x28], rcx    # ctx->r14 = 0
    mov     QWORD PTR [rax + 0x30], rcx    # ctx->r15 = 0
    mov     QWORD PTR [rax + 0x38], rdx    # ctx->rip = fn
    ret

jump_fcontext:
    # 分配上下文保存空间
    sub     rsp, 0x40

    # 保存寄存器
    mov     QWORD PTR [rsp + 0x08], rbp
    mov     QWORD PTR [rsp + 0x10], rbx
    mov     QWORD PTR [rsp + 0x18], r12
    mov     QWORD PTR [rsp + 0x20], r13
    mov     QWORD PTR [rsp + 0x28], r14
    mov     QWORD PTR [rsp + 0x30], r15

    # 保存返回地址和栈指针
    mov     rax, QWORD PTR [rsp + 0x40]
    mov     QWORD PTR [rsp + 0x38], rax    # save rip
    mov     QWORD PTR [rsp + 0x00], rsp    # save rsp

    # 设置返回值
    mov     rax, rsp                       # prev_ctx
    mov     rdx, rsi                       # userdata

    # 恢复目标上下文
    mov     rbp, QWORD PTR [rdi + 0x08]
    mov     rbx, QWORD PTR [rdi + 0x10]
    mov     r12, QWORD PTR [rdi + 0x18]
    mov     r13, QWORD PTR [rdi + 0x20]
    mov     r14, QWORD PTR [rdi + 0x28]
    mov     r15, QWORD PTR [rdi + 0x30]
    mov     rsp, QWORD PTR [rdi + 0x00]

    # 跳转执行
    jmp     QWORD PTR [rdi + 0x38]

.att_syntax prefix