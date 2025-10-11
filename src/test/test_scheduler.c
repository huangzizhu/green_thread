//
// Created by root on 2025/9/27.
//
//
// test_scheduler.c - 调度器测试和性能对比
//
#include "../../include/scheduler.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <assert.h>
#include <stdatomic.h>
#include <sys/syscall.h>
#include <dirent.h>
// 全局计数器用于测试
static atomic_int global_counter = 0;
static atomic_int task_completed = 0;

// 获取当前时间(微秒)
long long get_current_time_us() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000LL + tv.tv_usec;
}

// 简单的测试任务
void simple_task(void* arg) {
    int task_id = *(int*)arg;
    printf("任务 %d 开始执行\n", task_id);

    for (int i = 0; i < 3; i++) {
        printf("任务 %d: 步骤 %d\n", task_id, i + 1);
        atomic_fetch_add(&global_counter, 1);
    }

    printf("任务 %d 完成\n", task_id);
    atomic_fetch_add(&task_completed, 1);
}

// 协作式任务(会主动yield)
void cooperative_task(void* arg) {
    struct { int task_id; green_t* scheduler; } *params = arg;
    int task_id = params->task_id;
    green_t* scheduler = params->scheduler;

    printf("协作任务 %d 开始\n", task_id);

    for (int i = 0; i < 5; i++) {
        printf("协作任务 %d: 步骤 %d\n", task_id, i + 1);
        atomic_fetch_add(&global_counter, 1);

        if (i < 4) { // 最后一次不yield
            green_yield();
        }
    }

    printf("协作任务 %d 完成\n", task_id);
    atomic_fetch_add(&task_completed, 1);
}

// 测试1: 基本调度功能
void test_basic_scheduling() {
    printf("=== 基本调度功能测试 ===\n");

    green_t* scheduler = create_scheduler();
    assert(scheduler != NULL);

    // 重置计数器
    atomic_store(&global_counter, 0);
    atomic_store(&task_completed, 0);

    // 创建测试任务
    int task_ids[] = {1, 2, 3};
    for (int i = 0; i < 3; i++) {
        uint64_t tid = add_task(simple_task, &task_ids[i]);
        printf("创建任务 %d, TID: %lu\n", task_ids[i], tid);
        assert(tid != 0);
    }

    printf("开始调度...\n");
    green_run();

    // 验证结果
    assert(atomic_load(&task_completed) == 3);
    assert(atomic_load(&global_counter) == 9); // 3个任务 × 3次计数

    printf("✓ 基本调度功能测试通过\n\n");
}

// 测试2: 协作式调度
void test_cooperative_scheduling() {
    printf("=== 协作式调度测试 ===\n");

    green_t* scheduler = create_scheduler();

    // 重置计数器
    atomic_store(&global_counter, 0);
    atomic_store(&task_completed, 0);

    // 创建协作式任务
    struct { int task_id; green_t* scheduler; } params[] = {
        {1, scheduler}, {2, scheduler}, {3, scheduler}
    };

    for (int i = 0; i < 3; i++) {
        uint64_t tid = add_task(cooperative_task, &params[i]);
        printf("创建协作任务 %d, TID: %lu\n", params[i].task_id, tid);
    }

    printf("开始协作式调度...\n");
    green_run();

    // 验证结果
    assert(atomic_load(&task_completed) == 3);
    assert(atomic_load(&global_counter) == 15); // 3个任务 × 5次计数

    printf("✓ 协作式调度测试通过\n\n");
}

// 测试3: 压力测试
void test_stress_scheduling() {
    printf("=== 调度器压力测试 ===\n");

    const int TASK_COUNT = 1000;
    green_t* scheduler = create_scheduler();

    // 重置计数器
    atomic_store(&task_completed, 0);

    int* task_ids = malloc(sizeof(int) * TASK_COUNT);
    for (int i = 0; i < TASK_COUNT; i++) {
        task_ids[i] = i;
    }

    printf("创建 %d 个任务...\n", TASK_COUNT);
    long long start_time = get_current_time_us();

    for (int i = 0; i < TASK_COUNT; i++) {
        uint64_t tid = add_task(simple_task, &task_ids[i]);
        assert(tid != 0);
    }

    long long create_time = get_current_time_us() - start_time;

    printf("开始调度 %d 个任务...\n", TASK_COUNT);
    start_time = get_current_time_us();

    green_run();

    long long schedule_time = get_current_time_us() - start_time;

    // 验证结果
    assert(atomic_load(&task_completed) == TASK_COUNT);

    printf("✓ 压力测试通过\n");
    printf("  创建%d个任务耗时: %lld us (%.2f us/task)\n",
           TASK_COUNT, create_time, (double)create_time / TASK_COUNT);
    printf("  调度%d个任务耗时: %lld us (%.2f us/task)\n",
           TASK_COUNT, schedule_time, (double)schedule_time / TASK_COUNT);

    free(task_ids);
    printf("\n");
}
static void print_self_threads(void)
{
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/task", getpid());
    DIR *d = opendir(path);
    if (!d) return;
    int cnt = -1;                  // 排除 “.” 目录
    while (readdir(d)) cnt++;
    closedir(d);
    printf("=== 内核线程数: %d ===\n", cnt);
}




// 主测试函数
int main() {
    printf("开始调度器测试...\n\n");

    test_basic_scheduling();
    test_cooperative_scheduling();
    test_stress_scheduling();
    destroy_scheduler();
    printf("所有测试完成! 🎉\n");
    printf("\n建议:\n");
    printf("1. 使用 valgrind 检查内存泄露\n");
    printf("2. 使用 perf 进行更详细的性能分析\n");
    printf("3. 在不同负载下多次运行性能测试\n");
    print_self_threads();
    return 0;
}