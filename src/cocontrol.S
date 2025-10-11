# x86-64 SysV ABI 有栈协程上下文切换原语
# 完全兼容 boost.context 的 make_fcontext / jump_fcontext

.text
.global make_fcontext
.global jump_fcontext
.type make_fcontext, @function
.type jump_fcontext, @function

# 上下文结构布局（相对偏移）：
# rsp: 0x00 (0)   - 栈指针
# rbp: 0x08 (8)   - 帧指针
# rbx: 0x10 (16)  - 通用寄存器
# r12: 0x18 (24)  - 通用寄存器
# r13: 0x20 (32)  - 通用寄存器
# r14: 0x28 (40)  - 通用寄存器
# r15: 0x30 (48)  - 通用寄存器
# rip: 0x38 (56)  - 指令指针（返回地址）
# 总大小: 0x40 (64) 字节

#==============================================================================
# void* make_fcontext(void* stack_top, size_t stack_size, void (*fn)(transfer_t))
#==============================================================================
# 参数: rdi = stack_top (栈顶高地址)
#       rsi = stack_size (栈大小，未使用)
#       rdx = fn (入口函数指针)
# 返回: rax = fcontext_t (上下文指针)

make_fcontext:
    # 从栈顶预留 0x40 字节作为上下文保存区
    leaq    -0x40(%rdi), %rax       # rax = stack_top - 64

    leaq    -0x100(%rdi), %rcx        # 预留256字节空间
    andq    $-16, %rcx              # 16字节对齐
    addq    $-8, %rcx                  # ✅ 故意偏移 8 字节，尾号变成 8
    # ✅ 新增：在 rcx+0x40 位置预置返回地址
    # 这个位置相当于 "call" 指令压入的返回地址位置
    movq    %rdx, 0x40(%rcx)        # 预置函数入口地址

    # 初始化上下文结构
    movq    %rcx, 0x00(%rax)        # ctx->rsp = aligned_stack_top
    xorq    %rcx, %rcx              # 清零寄存器
    movq    %rcx, 0x08(%rax)        # ctx->rbp = 0
    movq    %rcx, 0x10(%rax)        # ctx->rbx = 0
    movq    %rcx, 0x18(%rax)        # ctx->r12 = 0
    movq    %rcx, 0x20(%rax)        # ctx->r13 = 0
    movq    %rcx, 0x28(%rax)        # ctx->r14 = 0
    movq    %rcx, 0x30(%rax)        # ctx->r15 = 0
    movq    %rdx, 0x38(%rax)        # ctx->rip = fn (入口函数)

    ret                             # 返回 fcontext_t

#==============================================================================
# transfer_t jump_fcontext(fcontext_t const to, void* userdata)
#==============================================================================
# 参数: rdi = to (目标上下文指针)
#       rsi = userdata (用户数据)
# 返回: rax = transfer_t.prev_ctx (前一个上下文)
#       rdx = transfer_t.userdata (用户数据)

jump_fcontext:
    # 在当前栈上分配 0x40 字节保存当前上下文
    subq    $0x40, %rsp

    # 保存 callee-saved 寄存器到当前栈
    movq    %rbp, 0x08(%rsp)        # 保存 rbp
    movq    %rbx, 0x10(%rsp)        # 保存 rbx
    movq    %r12, 0x18(%rsp)        # 保存 r12
    movq    %r13, 0x20(%rsp)        # 保存 r13
    movq    %r14, 0x28(%rsp)        # 保存 r14
    movq    %r15, 0x30(%rsp)        # 保存 r15

    # 保存返回地址（从调用者栈帧获取）
    movq    0x40(%rsp), %rax        # 获取原返回地址
    movq    %rax, 0x38(%rsp)        # 保存到 ctx->rip

    # 保存当前栈指针
    movq    %rsp, 0x00(%rsp)        # 保存 ctx->rsp

    # 设置返回值（在切换上下文之前）
    movq    %rsp, %rax              # transfer_t.prev_ctx = 当前上下文
    movq    %rsi, %rdx              # transfer_t.userdata = userdata

    # 从目标上下文恢复 callee-saved 寄存器
    movq    0x08(%rdi), %rbp        # 恢复 rbp
    movq    0x10(%rdi), %rbx        # 恢复 rbx
    movq    0x18(%rdi), %r12        # 恢复 r12
    movq    0x20(%rdi), %r13        # 恢复 r13
    movq    0x28(%rdi), %r14        # 恢复 r14
    movq    0x30(%rdi), %r15        # 恢复 r15
    movq    0x00(%rdi), %rsp        # 恢复 rsp（切换栈）

    # 跳转到目标执行点
    jmpq    *0x38(%rdi)             # 跳转到 ctx->rip