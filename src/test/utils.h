//
// Created by root on 2026/3/9.
//

#ifndef GREEN_THREAD_UTILS_H
#define GREEN_THREAD_UTILS_H
#include <stdio.h>
#include <time.h>



// 初始化锚点：记录程序启动时的单调时钟
void init_time_anchor();

// 获取相对于锚点的微秒数（返回long类型，避免负数）
long get_elapsed_us();
void print_time();
#endif //GREEN_THREAD_UTILS_H