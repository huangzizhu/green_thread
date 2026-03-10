//
// Created by root on 2025/10/13.
//

#ifndef GREEN_THREAD_CHANNEL_H
#define GREEN_THREAD_CHANNEL_H
#include <stddef.h>
typedef struct channel* channel_t;

//创建一个容量为cap的通道，每个元素大小为elem_size，当cap为0时表示无缓冲通道
channel_t create_channel(size_t elem_size, size_t cap);
void send_channel(channel_t channel, void* data);
void recv_channel(channel_t channel, void* data);
//返回值 1 成功销毁， 0 失败（如队列非空），-1 参数错误
int destroy_channel(channel_t channel);



#endif //GREEN_THREAD_CHANNEL_H