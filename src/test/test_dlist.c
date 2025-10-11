//
// Created by root on 2025/9/27.
//
//
// test_dlist.c - 双向链表测试
//
#include "dlist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>

// 测试用的数据结构
typedef struct test_data {
    int value;
    char name[32];
    dlist_node node;
} test_data_t;

// 创建测试数据
test_data_t* create_test_data(int value, const char* name) {
    test_data_t* data = malloc(sizeof(test_data_t));
    if (!data) return NULL;

    data->value = value;
    strncpy(data->name, name, sizeof(data->name) - 1);
    data->name[sizeof(data->name) - 1] = '\0';
    node_init(&data->node);
    return data;
}

// 释放测试数据
void free_test_data(test_data_t* data) {
    if (data) free(data);
}

// 获取当前时间(微秒)
long long get_current_time_us() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000LL + tv.tv_usec;
}

// 测试1: 基本功能测试
void test_basic_operations() {
    printf("=== 测试基本功能 ===\n");

    dlist* list = create_dlist();
    assert(list != NULL);
    assert(dlist_empty(list) == 1);
    assert(len(list) == 0);

    // 测试插入到头部
    test_data_t* data1 = create_test_data(1, "first");
    test_data_t* data2 = create_test_data(2, "second");
    test_data_t* data3 = create_test_data(3, "third");

    assert(insert_head(list, &data1->node) == 0);
    assert(len(list) == 1);
    assert(!dlist_empty(list));

    assert(insert_head(list, &data2->node) == 0);
    assert(len(list) == 2);

    assert(insert_tail(list, &data3->node) == 0);
    assert(len(list) == 3);

    // 测试获取头尾节点
    dlist_node* head_node = get_head(list);
    test_data_t* head_data = get_data(head_node, test_data_t, node);
    assert(head_data->value == 2); // data2应该在头部

    dlist_node* tail_node = get_tail(list);
    test_data_t* tail_data = get_data(tail_node, test_data_t, node);
    assert(tail_data->value == 3); // data3应该在尾部

    // 测试删除
    dlist_node* removed = remove_head(list);
    test_data_t* removed_data = get_data(removed, test_data_t, node);
    assert(removed_data->value == 2);
    assert(len(list) == 2);

    removed = remove_tail(list);
    removed_data = get_data(removed, test_data_t, node);
    assert(removed_data->value == 3);
    assert(len(list) == 1);
    destroy_dlist(list);
    free_test_data(data1);
    free_test_data(data2);
    free_test_data(data3);


    printf("✓ 基本功能测试通过\n");
}

// 测试2: 边界条件测试
void test_edge_cases() {
    printf("=== 测试边界条件 ===\n");

    dlist* list = create_dlist();

    // 空链表操作
    assert(remove_head(list) == NULL);
    assert(remove_tail(list) == NULL);
    assert(get_head(list) == NULL);
    assert(get_tail(list) == NULL);

    // 单元素链表
    test_data_t* data = create_test_data(42, "single");
    insert_head(list, &data->node);

    assert(get_head(list) == get_tail(list));
    assert(len(list) == 1);

    dlist_node* removed = remove_head(list);
    assert(removed == &data->node);
    assert(dlist_empty(list));
    assert(get_tail(list) == NULL);

    // 测试NULL参数
    assert(insert_head(NULL, &data->node) == -2);
    assert(insert_head(list, NULL) == -2);
    assert(insert_tail(NULL, &data->node) == -2);
    assert(insert_tail(list, NULL) == -2);
    destroy_dlist(list);
    free_test_data(data);


    printf("✓ 边界条件测试通过\n");
}

// 测试3: 大量操作压力测试
void test_stress_operations() {
    printf("=== 压力测试 ===\n");

    const int TEST_SIZE = 100000;
    dlist* list = create_dlist();
    test_data_t** test_items = malloc(sizeof(test_data_t*) * TEST_SIZE);

    long long start_time = get_current_time_us();

    // 大量插入操作
    printf("插入 %d 个元素...\n", TEST_SIZE);
    for (int i = 0; i < TEST_SIZE; i++) {
        char name[32];
        snprintf(name, sizeof(name), "item_%d", i);
        test_items[i] = create_test_data(i, name);

        if (i % 2 == 0) {
            insert_head(list, &test_items[i]->node);
        } else {
            insert_tail(list, &test_items[i]->node);
        }

        // 每1000次检查一下链表长度
        if ((i + 1) % 1000 == 0) {
            assert(len(list) == i + 1);
        }
    }

    assert(len(list) == TEST_SIZE);
    long long insert_time = get_current_time_us() - start_time;

    // 验证链表完整性
    printf("验证链表完整性...\n");
    int count = 0;
    dlist_node* current = get_head(list);
    while (current && count < TEST_SIZE + 10) { // 防止死循环
        test_data_t* data = get_data(current, test_data_t, node);
        assert(data->value >= 0 && data->value < TEST_SIZE);

        if (current == get_tail(list)) break;
        current = current->next;
        if (current == &list->head) break; // 到达链表头，循环完成
        count++;
    }

    // 大量删除操作
    printf("删除所有元素...\n");
    start_time = get_current_time_us();
    int removed_count = 0;
    while (!dlist_empty(list)) {
        dlist_node* node;
        if (removed_count % 2 == 0) {
            node = remove_head(list);
        } else {
            node = remove_tail(list);
        }
        assert(node != NULL);
        removed_count++;
    }

    assert(removed_count == TEST_SIZE);
    assert(len(list) == 0);
    long long remove_time = get_current_time_us() - start_time;
    destroy_dlist(list);
    // 释放内存
    for (int i = 0; i < TEST_SIZE; i++) {
        free_test_data(test_items[i]);
    }
    free(test_items);


    printf("✓ 压力测试通过\n");
    printf("  插入%d个元素耗时: %lld us (%.2f us/op)\n",
           TEST_SIZE, insert_time, (double)insert_time / TEST_SIZE);
    printf("  删除%d个元素耗时: %lld us (%.2f us/op)\n",
           TEST_SIZE, remove_time, (double)remove_time / TEST_SIZE);
}

// 测试4: 内存泄露检测
void test_memory_leak() {
    printf("=== 内存泄露测试 ===\n");

    // 多次创建和销毁链表
    for (int round = 0; round < 1000; round++) {
        dlist* list = create_dlist();

        // 添加一些元素
        for (int i = 0; i < 10; i++) {
            test_data_t* data = create_test_data(i, "leak_test");
            insert_head(list, &data->node);
        }

        // 删除所有元素
        while (!dlist_empty(list)) {
            dlist_node* node = remove_head(list);
            test_data_t* data = get_data(node, test_data_t, node);
            free_test_data(data);
        }

        destroy_dlist(list);
    }

    printf("✓ 内存泄露测试完成 (请使用valgrind验证)\n");
}

// 主测试函数
int main() {
    printf("开始双向链表测试...\n\n");

    test_basic_operations();
    test_edge_cases();
    test_stress_operations();
    test_memory_leak();

    printf("\n所有测试通过! 🎉\n");
    printf("建议使用以下命令检查内存泄露:\n");
    printf("valgrind --leak-check=full --track-origins=yes ./test_dlist\n");

    return 0;
}