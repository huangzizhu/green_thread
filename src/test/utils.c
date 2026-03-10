//
// Created by root on 2026/3/9.
//

#include "utils.h"
#include <stdio.h>
#include <time.h>

// 全局变量存储锚点时间（程序启动时的单调时钟值）
static struct timespec anchor_ts;

// 初始化锚点：记录程序启动时的单调时钟
void init_time_anchor() {
    // CLOCK_MONOTONIC：单调时钟，从系统启动开始计时，不受系统时间修改影响
    if (clock_gettime(CLOCK_MONOTONIC, &anchor_ts) == -1) {
        perror("clock_gettime failed");
        return;
    }
}

// 获取相对于锚点的微秒数（返回long类型，避免负数）
long get_elapsed_us() {
    struct timespec current_ts;
    if (clock_gettime(CLOCK_MONOTONIC, &current_ts) == -1) {
        perror("clock_gettime failed");
        return -1;
    }

    // 计算时间差（秒转微秒 + 纳秒转微秒）
    long sec_diff = (current_ts.tv_sec - anchor_ts.tv_sec) * 1000000L;
    long nsec_diff = (current_ts.tv_nsec - anchor_ts.tv_nsec) / 1000L;  // 纳秒转微秒
    return sec_diff + nsec_diff;
}
void print_time()
{
    printf("\ttimestamp: %ld us\n", get_elapsed_us());
}