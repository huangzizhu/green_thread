//
// Created by root on 25-9-27.
//

#ifndef SCHEDULER_H
#define SCHEDULER_H
#include <stdint.h>
#include "dlist.h"
#include <setjmp.h>

enum {STACK_SIZE = 256 * 1024u};

typedef void(*task_fuc_t)(void* arg);

typedef struct green_thread_scheduler green_t;
void green_yield();
uint64_t add_task(task_fuc_t fuc, void* arg);
green_t* create_scheduler();
void destroy_scheduler();
void green_run();
#endif //SCHEDULER_H
