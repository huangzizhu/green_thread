//
// Created by root on 25-9-27.
//

#ifndef SCHEDULER_H
#define SCHEDULER_H
#include <stdint.h>
#include "dlist.h"
#include <setjmp.h>

#include "context.h"

enum {STACK_SIZE = 256 * 1024u};
typedef enum { ST_DEAD, ST_RUNNING, ST_READY, ST_BLOCK } tcb_state;
typedef void(*task_fuc_t)(void* arg);
typedef struct task_control_block tcb_t;
typedef struct green_thread_scheduler green_t;
void add_ready_task(tcb_t* tcb);
void green_yield();
uint64_t add_task(task_fuc_t fuc, void* arg);
green_t* create_scheduler();
void destroy_scheduler();
void green_run();
tcb_t* get_curr();
void set_tcb_state(tcb_t* tcb, tcb_state state);
fcontext_t get_scheduler_ctx();
#endif //SCHEDULER_H
