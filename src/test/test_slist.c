//
// Created by root on 2026/3/11.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../../include/slist.h"

// 测试辅助函数
#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            printf("FAIL: %s:%d: %s\n", __FILE__, __LINE__, msg); \
            return 0; \
        } \
    } while(0)

#define TEST_PASS(msg) \
    printf("PASS: %s\n", msg)

// 测试用例结构
typedef struct {
    const char* name;
    int (*func)(void);
} test_case_t;

// ==================== 测试函数 ====================

// 测试1: 基本创建和销毁
int test_create_destroy() {
    printf("Test: create_destroy\n");

    // 正常创建
    slist_t* list = slist_create();
    TEST_ASSERT(list != NULL, "List should be created");
    TEST_ASSERT(list->size == 0, "New list size should be 0");
    TEST_ASSERT(list->head == NULL, "New list head should be NULL");

    // 销毁
    slist_destroy(list);

    // 多次创建销毁
    for (int i = 0; i < 100; i++) {
        list = slist_create();
        TEST_ASSERT(list != NULL, "List creation should succeed");
        slist_destroy(list);
    }

    TEST_PASS("create_destroy test passed");
    return 1;
}

// 测试2: 空列表操作
int test_empty_list_operations() {
    printf("Test: empty_list_operations\n");

    slist_t* list = slist_create();

    // 测试空列表的is_empty
    TEST_ASSERT(slist_is_empty(list) == 1, "New list should be empty");

    // 从空列表移除头部
    void* data = slist_remove_head(list);
    TEST_ASSERT(data == NULL, "Removing from empty list should return NULL");
    TEST_ASSERT(list->size == 0, "Size should remain 0");

    // 销毁空列表
    slist_destroy(list);

    TEST_PASS("empty_list_operations test passed");
    return 1;
}

// 测试3: 插入和移除单个元素
int test_single_element() {
    printf("Test: single_element\n");

    slist_t* list = slist_create();
    int test_data = 42;

    // 插入单个元素
    slist_insert_head(list, &test_data);
    TEST_ASSERT(list->size == 1, "Size should be 1");
    TEST_ASSERT(slist_is_empty(list) == 0, "List should not be empty");
    TEST_ASSERT(list->head != NULL, "Head should not be NULL");
    TEST_ASSERT(list->head->data == &test_data, "Data should match");

    // 移除单个元素
    int* retrieved = (int*)slist_remove_head(list);
    TEST_ASSERT(retrieved == &test_data, "Should retrieve the same data");
    TEST_ASSERT(list->size == 0, "Size should be 0");
    TEST_ASSERT(list->head == NULL, "Head should be NULL");
    TEST_ASSERT(slist_is_empty(list) == 1, "List should be empty");

    slist_destroy(list);

    TEST_PASS("single_element test passed");
    return 1;
}

// 测试4: 多个元素操作
int test_multiple_elements() {
    printf("Test: multiple_elements\n");

    slist_t* list = slist_create();
    int data[100];

    // 插入100个元素
    for (int i = 0; i < 100; i++) {
        data[i] = i;
        slist_insert_head(list, &data[i]);
        TEST_ASSERT(list->size == i + 1, "Size should increment");
    }

    // 验证LIFO顺序（头插法）
    for (int i = 99; i >= 0; i--) {
        int* retrieved = (int*)slist_remove_head(list);
        TEST_ASSERT(*retrieved == i, "Should retrieve in reverse order");
        TEST_ASSERT(list->size == i, "Size should decrement");
    }

    TEST_ASSERT(slist_is_empty(list) == 1, "List should be empty");

    slist_destroy(list);

    TEST_PASS("multiple_elements test passed");
    return 1;
}

// 测试5: 高强度插入移除
int test_stress_operations() {
    printf("Test: stress_operations\n");

    slist_t* list = slist_create();
    const int iterations = 10000;

    // 高强度交替插入移除
    for (int i = 0; i < iterations; i++) {
        int* data = malloc(sizeof(int));
        *data = i;
        slist_insert_head(list, data);

        if (i % 3 == 0) {  // 每3次操作移除一次
            int* removed = slist_remove_head(list);
            free(removed);
        }
    }

    // 清理剩余数据
    while (!slist_is_empty(list)) {
        int* data = slist_remove_head(list);
        free(data);
    }

    slist_destroy(list);

    TEST_PASS("stress_operations test passed");
    return 1;
}

// 测试6: 边界值测试
int test_boundary_values() {
    printf("Test: boundary_values\n");

    slist_t* list = slist_create();

    // 测试NULL数据
    slist_insert_head(list, NULL);
    TEST_ASSERT(list->size == 1, "Should accept NULL data");

    void* data = slist_remove_head(list);
    TEST_ASSERT(data == NULL, "Should retrieve NULL");

    // 测试大内存数据
    size_t large_size = 1024 * 1024;  // 1MB
    void* large_data = malloc(large_size);
    memset(large_data, 0xFF, large_size);

    slist_insert_head(list, large_data);
    void* retrieved = slist_remove_head(list);
    TEST_ASSERT(retrieved == large_data, "Should retrieve large data");
    free(large_data);

    slist_destroy(list);

    TEST_PASS("boundary_values test passed");
    return 1;
}

// 测试7: 数据类型测试
int test_data_types() {
    printf("Test: data_types\n");

    slist_t* list = slist_create();

    // 测试不同数据类型
    int int_val = 123;
    float float_val = 3.14f;
    double double_val = 2.71828;
    char char_val = 'A';
    char str_val[] = "Hello, World!";

    slist_insert_head(list, &int_val);
    slist_insert_head(list, &float_val);
    slist_insert_head(list, &double_val);
    slist_insert_head(list, &char_val);
    slist_insert_head(list, str_val);

    TEST_ASSERT(list->size == 5, "Should have 5 elements");

    // 验证顺序和值
    char* str_retrieved = slist_remove_head(list);
    TEST_ASSERT(strcmp(str_retrieved, str_val) == 0, "String should match");

    char* char_retrieved = slist_remove_head(list);
    TEST_ASSERT(*char_retrieved == char_val, "Char should match");

    double* double_retrieved = slist_remove_head(list);
    TEST_ASSERT(*double_retrieved == double_val, "Double should match");

    float* float_retrieved = slist_remove_head(list);
    TEST_ASSERT(*float_retrieved == float_val, "Float should match");

    int* int_retrieved = slist_remove_head(list);
    TEST_ASSERT(*int_retrieved == int_val, "Int should match");

    slist_destroy(list);

    TEST_PASS("data_types test passed");
    return 1;
}

// 测试8: 并发模式测试（虽然不是线程安全，但测试交替操作）
int test_interleaved_operations() {
    printf("Test: interleaved_operations\n");

    slist_t* list1 = slist_create();
    slist_t* list2 = slist_create();

    int data1[50];
    int data2[50];

    // 交替操作两个列表
    for (int i = 0; i < 50; i++) {
        data1[i] = i;
        data2[i] = i + 1000;

        slist_insert_head(list1, &data1[i]);
        slist_insert_head(list2, &data2[i]);

        if (i % 5 == 0) {
            slist_remove_head(list1);
            slist_remove_head(list2);
        }
    }

    // 清理
    while (!slist_is_empty(list1)) {
        slist_remove_head(list1);
    }

    while (!slist_is_empty(list2)) {
        slist_remove_head(list2);
    }

    slist_destroy(list1);
    slist_destroy(list2);

    TEST_PASS("interleaved_operations test passed");
    return 1;
}

// 测试9: 内存泄漏检测辅助测试
int test_memory_leak_scenario() {
    printf("Test: memory_leak_scenario\n");

    // 这个测试特别设计来检查内存泄漏
    // 分配动态内存并插入到列表中
    slist_t* list = slist_create();

    const int num_elements = 100;
    int** allocated_ptrs = malloc(num_elements * sizeof(int*));

    // 分配内存并插入
    for (int i = 0; i < num_elements; i++) {
        allocated_ptrs[i] = malloc(sizeof(int));
        *allocated_ptrs[i] = i;
        slist_insert_head(list, allocated_ptrs[i]);
    }

    // 移除但不释放（模拟内存泄漏）
    for (int i = 0; i < num_elements; i++) {
        void* data = slist_remove_head(list);
        free(data);
    }

    // 释放分配器数组
    free(allocated_ptrs);

    // 注意：这里我们故意不释放从列表中移除的数据
    // 这将在valgrind中产生内存泄漏报告

    slist_destroy(list);

    TEST_PASS("memory_leak_scenario test passed (check valgrind output)");
    return 1;
}

// 测试10: 极端边界情况
int test_extreme_cases() {
    printf("Test: extreme_cases\n");

    // 测试0大小操作
    slist_t* list = slist_create();

    // 快速插入移除大量元素
    for (int i = 0; i < 1000; i++) {
        int val = i;
        slist_insert_head(list, &val);
        void* data = slist_remove_head(list);
        TEST_ASSERT(*(int*)data == i, "Should get same value back");
    }

    // 测试空指针参数（需要修改slist.h使其有错误处理）
    // 注意：当前实现可能崩溃，这是预期的
    // slist_insert_head(NULL, NULL);  // 这应该崩溃或返回错误

    slist_destroy(list);

    TEST_PASS("extreme_cases test passed");
    return 1;
}

// ==================== 测试运行器 ====================

test_case_t test_cases[] = {
    {"create_destroy", test_create_destroy},
    {"empty_list_operations", test_empty_list_operations},
    {"single_element", test_single_element},
    {"multiple_elements", test_multiple_elements},
    {"stress_operations", test_stress_operations},
    {"boundary_values", test_boundary_values},
    {"data_types", test_data_types},
    {"interleaved_operations", test_interleaved_operations},
    {"memory_leak_scenario", test_memory_leak_scenario},
    {"extreme_cases", test_extreme_cases},
    {NULL, NULL}
};

int main() {
    printf("Starting slist test suite...\n");
    printf("============================\n\n");

    int passed = 0;
    int total = 0;

    for (int i = 0; test_cases[i].name != NULL; i++) {
        total++;
        if (test_cases[i].func()) {
            passed++;
        } else {
            printf("Test '%s' failed!\n", test_cases[i].name);
        }
        printf("\n");
    }

    printf("============================\n");
    printf("Test Summary: %d/%d tests passed\n", passed, total);

    if (passed == total) {
        printf("All tests passed successfully!\n");
        return 0;
    } else {
        printf("Some tests failed!\n");
        return 1;
    }
}