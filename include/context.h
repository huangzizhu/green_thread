//
// Created by root on 2025/9/28.
//

#ifndef GREEN_THREAD_CONTEXT_H
#define GREEN_THREAD_CONTEXT_H
#include <stddef.h>

typedef void* fcontext_t;

typedef struct {
    fcontext_t ctx;
    void      *data;
} transfer_t;

#ifdef __cplusplus
extern "C" {
#endif

    fcontext_t make_fcontext(void *stack_top, size_t stack_size, void (*fn)());
    transfer_t jump_fcontext(fcontext_t const to, void *userdata);

#ifdef __cplusplus
}
#endifEN_THREAD_CONTEXT_H

#endif //GREEN_THREAD_CONTEXT_H
#endif