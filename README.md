
<!-- 0. 徽章区 -->
<p align="center">
  <img src="https://img.shields.io/badge/C-99-A8B9CC?style=flat-square&labelColor=34495e" alt="C99"/>
  <img src="https://img.shields.io/badge/POSIX-✔-1abc9c?style=flat-square&labelColor=34495e" alt="POSIX"/>
  <img src="https://img.shields.io/badge/ABI-✔-ff9f43?style=flat-square&labelColor=34495e" alt="ABI"/>
  <img src="https://img.shields.io/badge/version-1.0-ff6b6b?style=flat-square&labelColor=34495e" alt="Version"/>
  <img src="https://img.shields.io/badge/License-MIT-1abc9c?style=flat-square&labelColor=34495e" alt="License"/>
</p>

<h3 align="center">GreenThread 🌿</h3>
<p align="center"><b>用户态协程库 | 仿 boost.context | 纯 C + 汇编 | ABI + POSIX 兼容</b></p>

---

## 📌 简介
**GreenThread** 是我自研的 **用户态协程（green thread）** 库，第一阶段（v1.0）仅用 **两个汇编函数** 完成上下文切换，即可实现：

- 创建调度器  `create_scheduler()`  
- 添加任务  `add_task()`  
- 启动调度  `green_run()`  
- 主动让出  `green_yield()`

全代码 **零依赖**，遵循 **System V ABI & POSIX**，可在 **Linux/x86_64** 上直接 `make` 运行。  
线程级全局变量（`__thread`）保存 **curr 协程 + 调度器地址**，保证 **TLS 级隔离**。

---

## 🧩 核心实现
| 文件 | 职责 |
|---|---|
| `include/context.h` | 上下文结构体（`fcontext_t`） |
| `include/scheduler.h` | 调度器 & 公开 API |
| `include/dlist.h` | 双向循环队列（就绪队列） |
| `src/cocontrol.S` | `make_context` & `jump_context`（AT&T 语法） |
| `src/cocontrol_intel.S` | Intel 语法版本，供切换（仅保留，大概率不兼容WIN） |
| `src/scheduler.c` | 调度循环、就绪队列、TLS 变量 |
| `src/dlist.c` | 队列基础实现 |
| `test/test_*.c` | 单元测试，包含调度器测试及链表测试 |

---

## 📸 运行截图
<details>
<summary>👉 点我展开调度器测试结果 / 收起</summary>


  === 基本调度功能测试 ===
  创建任务 1, TID: 7559996246098706432
  创建任务 2, TID: 7559996246098706433
  创建任务 3, TID: 7559996246098706434
  开始调度...
  任务 1 开始执行
  任务 1: 步骤 1
  任务 1: 步骤 2
  任务 1: 步骤 3
  任务 1 完成
  任务 2 开始执行
  任务 2: 步骤 1
  任务 2: 步骤 2
  任务 2: 步骤 3
  任务 2 完成
  任务 3 开始执行
  任务 3: 步骤 1
  任务 3: 步骤 2
  任务 3: 步骤 3
  任务 3 完成
  ✓ 基本调度功能测试通过
  
  === 协作式调度测试 ===
  创建协作任务 1, TID: 7559996246098706435
  创建协作任务 2, TID: 7559996246098706436
  创建协作任务 3, TID: 7559996246098706437
  开始协作式调度...
  协作任务 1 开始
  协作任务 1: 步骤 1
  协作任务 2 开始
  协作任务 2: 步骤 1
  协作任务 3 开始
  协作任务 3: 步骤 1
  协作任务 1: 步骤 2
  协作任务 2: 步骤 2
  协作任务 3: 步骤 2
  协作任务 1: 步骤 3
  协作任务 2: 步骤 3
  协作任务 3: 步骤 3
  协作任务 1: 步骤 4
  协作任务 2: 步骤 4
  协作任务 3: 步骤 4
  协作任务 1: 步骤 5
  协作任务 1 完成
  协作任务 2: 步骤 5
  协作任务 2 完成
  协作任务 3: 步骤 5
  协作任务 3 完成
  ✓ 协作式调度测试通过
  
  === 调度器压力测试 ===
  创建 1000 个任务...
  开始调度 1000 个任务...
  任务 0 开始执行
  任务 0: 步骤 1
  任务 0: 步骤 2
  任务 0: 步骤 3
  任务 0 完成
  任务 1 开始执行
  任务 1: 步骤 1
  任务 1: 步骤 2
  任务 1: 步骤 3
  。。。。
  任务 998 完成
  任务 999 开始执行
  任务 999: 步骤 1
  任务 999: 步骤 2
  任务 999: 步骤 3
  任务 999 完成
  ✓ 压力测试通过
    创建1000个任务耗时: 2630 us (2.63 us/task)
    调度1000个任务耗时: 32649 us (32.65 us/task)
  
  所有测试完成! 🎉

</details>

<details>
<summary>👉 点我展开调度器测试结果 / 收起</summary>
  dlist测试无内存bug
  root@DESKTOP-2HE5EG7:/home/green_thread/cmake-build-debug# valgrind --leak-check=full --track-origins=yes ./test_dlist
  ==180739== Memcheck, a memory error detector
  ==180739== Copyright (C) 2002-2017, and GNU GPL'd, by Julian Seward et al.
  ==180739== Using Valgrind-3.18.1 and LibVEX; rerun with -h for copyright info
  ==180739== Command: ./test_dlist
  ==180739== 
  开始双向链表测试...
  
  === 测试基本功能 ===
  ✓ 基本功能测试通过
  === 测试边界条件 ===
  ✓ 边界条件测试通过
  === 压力测试 ===
  插入 100000 个元素...
  验证链表完整性...
  删除所有元素...
  ✓ 压力测试通过
    插入100000个元素耗时: 510936 us (5.11 us/op)
    删除100000个元素耗时: 15071 us (0.15 us/op)
  === 内存泄露测试 ===
  ✓ 内存泄露测试完成 (请使用valgrind验证)
  
  所有测试通过! 🎉
  建议使用以下命令检查内存泄露:
  valgrind --leak-check=full --track-origins=yes ./test_dlist
  ==180739== 
  ==180739== HEAP SUMMARY:
  ==180739==     in use at exit: 0 bytes in 0 blocks
  ==180739==   total heap usage: 111,009 allocs, 111,009 frees, 6,993,344 bytes allocated
  ==180739== 
  ==180739== All heap blocks were freed -- no leaks are possible
  ==180739== 
  ==180739== For lists of detected and suppressed errors, rerun with: -s
  ==180739== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
</details>


## 📖 API 速查（v1.0）
```c
green_t* create_scheduler();
uint64_t add_task(task_fuc_t fuc, void* arg);
void green_run();
void green_yield();
```

---

## 🗺️ 目录树
```
.
├── CMakeLists.txt
├── include
│   ├── context.h
│   ├── dlist.h
│   └── scheduler.h
└── src
    ├── cocontrol.S
    ├── cocontrol_intel.S
    ├── dlist.c
    ├── scheduler.c
    └── test
        ├── test_dlist.c
        └── test_scheduler.c
```

---

## 🛠️ 下一步（v2.0 RoadMap）
- [ ] **Channel** 协程间通信（多生产者 / 单消费者模型）  
- [ ] **Select** 多路 channel 非阻塞读写  
- [ ] **IO 多路复用** + 阻塞链自动让出（epoll）  
- [ ] **Work-Stealing** 调度算法  
- [ ] **Benchmark** vs goroutine / libtask  

---

## 📝 博客 & 坑记
实战细节、ABI 踩坑、汇编调试笔记后续将在下方更新：  
👉 [【你的博客地址】](https://your-blog.com) （持续更新中）

---

