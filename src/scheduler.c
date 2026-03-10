//
// Created by root on 25-9-27.
//
#include "../include/scheduler.h"
#include "../include/context.h"
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/ucontext.h>
#include <pthread.h>
#include <unistd.h>
#include <stdalign.h>

#include "test/utils.h"


struct task_control_block
{
    tcb_state                                   state;//状态
    uint64_t                                    tid;//tid
    fcontext_t                                  ctx;//跳转上下文
    void*                                       stack;//分配的栈空间的地址+STACK_SIZE
    dlist_node                                  node;//嵌入式链表的指针
    task_fuc_t                                  func;//要调用的函数
    void*                                       arg;//参数
};

struct green_thread_scheduler
{
    dlist           ready_queue;//就绪队列
    fcontext_t      ctx;//调度器上下文
};
static __thread tcb_t* curr = NULL;//当前进行协程，线程级全局变量
static __thread green_t* scheduler = NULL;//线程唯一调度器，线程级全局变量
green_t* get_scheduler(){return scheduler;}
fcontext_t get_scheduler_ctx(){return scheduler->ctx;}
void set_scheduler(green_t* s){scheduler = s;}
tcb_t* get_curr() {return curr;}
void set_curr(tcb_t* t) {curr = t;}


void handle_no_scheduler()
{
    fprintf(stderr, "no scheduler\n");
    exit(-1);
}

void handle_no_curr()
{
    fprintf(stderr, "no scheduler\n");
    exit(-1);
}

void free_tcb(tcb_t* tcb)
{
    if (!tcb) return;
    munmap(tcb->stack - STACK_SIZE,STACK_SIZE);
    free(tcb);
}

void set_tcb_state(tcb_t* tcb, tcb_state state)
{
    if (!tcb) return;
    tcb->state = state;
}

tcb_t* pop_task()
{
    if (!scheduler) handle_no_scheduler();
    dlist_node* node = remove_head(&scheduler->ready_queue);
    if (node == NULL) return NULL;
    return get_data(node,tcb_t,node);
}
void push_task(tcb_t* task)
{
    if (!task) return;
    if (!scheduler) handle_no_scheduler();
    insert_tail(&scheduler->ready_queue,&task->node);
}

uint64_t generate_tid()
{
    static atomic_int t_count = 0;
    uint64_t high_time = (uint64_t)time(NULL) << 32;
    return (high_time | (atomic_fetch_add(&t_count,1) & 0xFFFFFFFF));
}

void* create_stack_space()
{
    void* stack_space = mmap(
        NULL,
        STACK_SIZE,
        PROT_READ | PROT_WRITE,
        MAP_ANONYMOUS | MAP_PRIVATE,
        -1,0);
    if (stack_space == MAP_FAILED)
    {
        fprintf(stderr, "mmap failed\n");
        exit(-1);
    }
    mprotect(stack_space, 4096, PROT_NONE);
    return stack_space + STACK_SIZE;
}

__attribute__((noinline, noreturn))
void coroutine_trampoline(void)   /* 故意不写参数 */
{
    fcontext_t prev;
    void      *arg;

    /* 一次汇编把 rax/rdx 钉死，并声明内存副作用 */
    __asm__ volatile (
        "movq %%rax, %0\n\t"
        "movq %%rdx, %1"
        : "=m"(prev), "=m"(arg)   // 强制写进内存
        :
        : "memory");

    transfer_t t = { .ctx = prev, .data = arg };

    scheduler->ctx = t.ctx;
    curr->state = ST_RUNNING;
    curr->func(curr->arg);
    curr->state = ST_DEAD;

    /**
     * 编译器将tcb优化进寄存器，等回来的时候tcb不见
     * 使用curr更好的方案
     */
    // tcb_t *tcb = curr;
    // set_curr(tcb);
    // scheduler->ctx = t.ctx;
    // tcb->state = ST_RUNNING;
    // tcb->func(tcb->arg);
    // tcb->state = ST_DEAD;

    jump_fcontext(scheduler->ctx, NULL);
    __builtin_unreachable();
}


uint64_t add_task(task_fuc_t fuc, void* arg)
{
    if (!scheduler) handle_no_scheduler();
    if (!scheduler || !fuc) return 0;
    // ReSharper disable once CppDFAMemoryLeak
    tcb_t* tcb = malloc(sizeof(tcb_t));
    if (tcb == NULL)
    {
        fprintf(stderr, "malloc failed\n");
        exit(-1);
    }
    //tcb初始化
    node_init(&tcb->node);
    tcb->arg = arg;
    tcb->tid = generate_tid();
    tcb->stack = create_stack_space();
    tcb->state = ST_READY;
    tcb->func = fuc;
    push_task(tcb);
    tcb->ctx = make_fcontext(tcb->stack,STACK_SIZE,coroutine_trampoline);
    // ReSharper disable once CppDFAMemoryLeak
    return tcb->tid;
}
__attribute__((noinline))
void green_yield()
{
    if (!scheduler) handle_no_scheduler();
    // ReSharper disable once CppDFANullDereference
    if (!curr) handle_no_curr();

    unsigned long caller_rbp;
    /* 从调用者帧抓 rbp：*$rbp */
    asm volatile("movq (%%rbp), %0" : "=r"(caller_rbp));

    /* 写回存档区 */
    *(uint64_t *)(curr->ctx + 0x00) = (uint64_t)__builtin_frame_address(0); /* rsp */
    *(uint64_t *)(curr->ctx + 0x08) = caller_rbp;                           /* 调用者 rbp */
    *(uint64_t *)(curr->ctx + 0x38) = (uint64_t)__builtin_return_address(0); /* rip */
    jump_fcontext(scheduler->ctx,NULL);
}

green_t* create_scheduler()
{
    if (!scheduler)
    {
        scheduler = malloc(sizeof(green_t));
        if (!scheduler)
        {
            fprintf(stderr, "malloc failed\n");
            exit(-1);
        }
    }
    else return scheduler;
    dlist_init(&scheduler->ready_queue);
    set_curr(NULL);
    return scheduler;
}


void destroy_scheduler()
{
    if (!scheduler) handle_no_scheduler();
    if (dlist_empty(&scheduler->ready_queue)) free(scheduler);//空的正常销毁，非空静默。
}

int next_task()
{
    //开始调度
    curr = pop_task();
    if (curr)
    {
        jump_fcontext(curr->ctx,curr);
        return 1;
    }
    return 0;//空调度器，返回
}

void green_run()
{
    if (!scheduler) handle_no_scheduler();
    unsigned long saved_ret = *(unsigned long *)__builtin_frame_address(0);
    for (;;)
    {
        if (next_task())//结束或者yield回来了
        {
            if (curr->state == ST_READY) push_task(curr);//task尚未结束且未阻塞
            else if (curr->state == ST_DEAD) free_tcb(curr);//已经结束
            //阻塞的task不放回就绪队列，等待外部事件唤醒
        }
        else
        {
            curr = NULL;
            *(unsigned long *)__builtin_frame_address(0) = saved_ret;
            return;
        }
    }
}

void add_ready_task(tcb_t* tcb)
{
    tcb->state = ST_READY;
    push_task(tcb);
}
