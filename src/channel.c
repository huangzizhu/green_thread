//
// Created by root on 2025/10/13.
//

#include "../include/channel.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "context.h"
#include "dlist.h"
#include "scheduler.h"

typedef struct rqueue
{
    size_t elem_size;
    size_t cap;
    size_t len;
    size_t head;
    size_t tail;
    char* data;
}rqueue_t;

typedef rqueue_t buf_t;

struct channel
{
    buf_t buf;
    dlist send_list;
    dlist recv_list;
};


typedef struct waiter
{
    tcb_t* tcb;
    char* data;
    dlist_node node;
}waiter_t;

//创建一个环形队列
rqueue_t* rq_new(size_t elem_size, size_t cap)
{
    void* data = calloc(cap,elem_size);
    if (!data)
    {
        fprintf(stderr, "calloc: out of memory\n");
        exit(-1);
    }
    rqueue_t* rqueue = calloc(1,sizeof(rqueue_t));
    if (!rqueue)
    {
        fprintf(stderr, "calloc: out of memory\n");
        exit(-1);
    }

    rqueue->data = data;
    rqueue->cap = cap;
    rqueue->elem_size = elem_size;
    return rqueue;
}
static inline void rq_destroy(rqueue_t* rqueue)
{
    if (rqueue->data)  free(rqueue->data);
}
//初始化已有的rq
void rq_init(rqueue_t* rqueue, size_t elem_size, size_t cap)
{
    void* data = calloc(cap,elem_size);
    if (!data)
    {
        fprintf(stderr, "calloc: out of memory\n");
        exit(-1);
    }
    rqueue->data = data;
    rqueue->cap = cap;
    rqueue->elem_size = elem_size;
    rqueue->len = 0;
    rqueue->head = 0;
    rqueue->tail = 0;
}
static inline int rq_empty(rqueue_t* rqueue){return rqueue->len == 0;}
static inline int rq_full(rqueue_t* rqueue){return rqueue->len == rqueue->cap;}

void rq_enqueue(rqueue_t* rqueue, void* data)
{
    if (!rqueue || !data) return;
    if (rq_full(rqueue))
    {
        fprintf(stderr, "rq_full\n");
        exit(-1);
    }
    memcpy(rqueue->data+rqueue->tail*rqueue->elem_size,data,rqueue->elem_size);
    rqueue->tail = (rqueue->tail + 1) % rqueue->cap;
    rqueue->len++;
}

static inline int is_buffer(channel_t chan){return chan->buf.cap != 0;}

void* rq_dequeue(rqueue_t* rqueue)
{
    if (!rqueue) return NULL;
    if (rq_empty(rqueue))
    {
        fprintf(stderr, "rq_empty\n");
        exit(-1);
    }
    void* data = rqueue->data+rqueue->head*rqueue->elem_size;
    rqueue->head = (rqueue->head + 1) % rqueue->cap;
    rqueue->len--;
    return data;
}

void add_waiter(dlist* list, void* data)
{
    waiter_t* w = malloc(sizeof(waiter_t));
    if (!w)
    {
        fprintf(stderr, "malloc: out of memory\n");
        exit(-1);
    }
    w->tcb = get_curr();
    w->data = data;
    set_tcb_state(w->tcb,ST_BLOCK);
    insert_tail(list,&w->node);
}

waiter_t* remove_waiter(dlist* list)
{
    dlist_node* node = remove_head(list);
    if (!node) return NULL;
    return get_data(node,waiter_t,node);
}


channel_t create_channel(size_t elem_size, size_t cap)
{
    channel_t channel = calloc(1,sizeof(struct channel));
    rq_init(&channel->buf,elem_size,cap);
    dlist_init(&channel->recv_list);
    dlist_init(&channel->send_list);
    return channel;
}

void send_channel(channel_t channel, void* data)
{
    if (!channel)
    {
        fprintf(stderr, "send_channel: channel is null\n");
        exit(-1);
    }
    if (!data)
    {
        fprintf(stderr, "send_channel: data is null\n");
        exit(-1);
    }
    if (is_buffer(channel))
    {
        //有缓
        if (rq_full(&channel->buf))
        {
            //满了阻塞
            add_waiter(&channel->send_list,data);
            //回到调度器
            green_yield();
            //唤醒后，recv在唤醒的时候已经帮忙塞进去了，直接离开
            return;
        }
        //没有满，交货
        rq_enqueue(&channel->buf,data);
        //如果有recv阻塞，唤醒一个
        if (!is_empty(&channel->recv_list))
        {
            waiter_t* recv = remove_waiter(&channel->recv_list);
            add_ready_task(recv->tcb);
            memcpy(recv->data,rq_dequeue(&channel->buf),channel->buf.elem_size);
            free(recv);
        }
        return;
    }
    //无缓冲
    if (is_empty(&channel->recv_list))
    {
        //如果没有recv，阻塞
        add_waiter(&channel->send_list,data);
        green_yield();
        return;
    }
    //有recv，交货
    waiter_t* recv = remove_waiter(&channel->recv_list);
    memcpy(recv->data,data,channel->buf.elem_size);
    add_ready_task(recv->tcb);
    free(recv);
}
void recv_channel(channel_t channel, void* data)
{
    if (!channel)
    {
        fprintf(stderr, "send_channel: channel is null\n");
        exit(-1);
    }
    if (!data)
    {
        fprintf(stderr, "send_channel: data is null\n");
        exit(-1);
    }
    if (is_buffer(channel))
    {
        //有缓冲
        if (rq_empty(&channel->buf))
        {
            //空的，阻塞
            add_waiter(&channel->recv_list,data);
            //回到调度器
            green_yield();
            return;
        }
        //不空，拿货
        void* buf_data = rq_dequeue(&channel->buf);
        memcpy(data,buf_data,channel->buf.elem_size);
        //如果有send阻塞，唤醒一个
        if (!is_empty(&channel->send_list))
        {
            waiter_t* send = remove_waiter(&channel->send_list);
            rq_enqueue(&channel->buf,send->data);
            add_ready_task(send->tcb);
            free(send);
        }
        return;
    }
    //无缓冲
    if (is_empty(&channel->send_list))
    {
        //如果没有发送者，阻塞
        add_waiter(&channel->recv_list,data);
        //回到调度器
        green_yield();
        return;
    }

   //有发送者，拿货
    waiter_t* send = remove_waiter(&channel->send_list);
    //复制数据
    memcpy(data,send->data,channel->buf.elem_size);
    //唤醒发送者
    add_ready_task(send->tcb);
    free(send);
}

//返回值 1 成功销毁， 0 失败（如队列非空），-1 参数错误
int destroy_channel(channel_t channel)
{
    if (!channel) return -1;
    int safe_to_destroy =
        is_empty(&channel->recv_list) &&
            is_empty(&channel->send_list) &&
                rq_empty(&channel->buf);
    if (safe_to_destroy)
    {
        //两个dlist都不能free
        //destroy_dlist(&channel->recv_list);
        //destroy_dlist(&channel->send_list);
        rq_destroy(&channel->buf);
        free(channel);
        return 1;
    }
    return 0;
}



