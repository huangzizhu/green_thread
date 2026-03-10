//
// Created by root on 2026/3/9.
//

#include "stdio.h"
#include "utils.h"
#include "./../../include/channel.h"
#include "./../../include/scheduler.h"
/*
 * 场景 1：无缓冲 channel - 先发后收
测试目的	验证无缓冲 channel 必须「发送 + 接收同时存在」才能完成通信
操作步骤
1. 创建无缓冲 channel（cap=0，elem_size=4）；
2. 启动协程 A：调用 send_channel 发送数据
3. 启动协程 B：调用 recv_channel 接收数据
4. 验证 A 和 B 都能成功完成通信
预期结果
1. 协程 A 不阻塞，数据成功从协程 A 拷贝到主线程接收缓冲区；
2. channel 的 send_list/recv_list 为空；
3. 无崩溃、无数据丢失。
 */


void coA(void* chan)
{
    channel_t channel = (channel_t) chan;
    int data = 42;
    printf("协程 A 准备发送数据: %d", data);
    print_time();
    send_channel(channel, &data);
    printf("协程 A 发送完成,解除阻塞");
    print_time();
}

void coB(void* chan)
{
    channel_t channel = (channel_t) chan;
    int recv_data;
    printf("协程 B 准备接收数据");
    print_time();
    recv_channel(channel, &recv_data);
    printf("协程 B 接收完成,数据: %d", recv_data);
    print_time();
}

void test01()
{
    printf("-------test01 start------------\n");
    channel_t channel = create_channel(4,0);
    add_task(coA, channel);
    add_task(coB,channel);
    green_run();
    printf("destroy 返回：%d\n", destroy_channel(channel));
    printf("-------test01 end------------\n");
}

/*
场景 2：无缓冲 channel - 先收后发
1. 启动协程 C：调用 recv_channel 数接收数据
2. 启动协程 D：调用 send_channel 发送数据
3. 验证 C 和 D 都能成功完成通信
 */
void coD(void* chan)
{
    channel_t channel = (channel_t) chan;
    int data = 42;
    printf("协程 D 准备发送数据: %d", data);
    print_time();
    send_channel(channel, &data);
    printf("协程 D 发送完成");
    print_time();
}

void coC(void* chan)
{
    channel_t channel = (channel_t) chan;
    int recv_data;
    printf("协程 C 准备接收数据");
    print_time();
    recv_channel(channel, &recv_data);
    printf("协程 C 接收完成,数据: %d,解除阻塞", recv_data);
    print_time();
}

void test02()
{
    printf("-------test02 start------------\n");
    channel_t channel = create_channel(4,0);
    add_task(coC, channel);
    add_task(coD,channel);
    green_run();
    printf("destroy 返回：%d\n", destroy_channel(channel));
    printf("-------test02 end------------\n");
}

/*
场景 3：有缓冲 channel - 发送不满缓冲
测试目的    验证有缓冲 channel 当缓冲未满时发送不阻塞
操作步骤
1. 创建有缓冲 channel（cap=3，elem_size=4）；
2. 启动协程 E：连续发送2个数据（缓冲3个）
3. 启动协程 F：接收1个数据
4. 启动协程 G：再发送1个数据
预期结果
1. 协程 E 发送2个数据不阻塞；
2. 协程 F 接收数据成功；
3. 协程 G 发送数据不阻塞；
4. 所有操作正常完成。
*/

void coE(void* chan)
{
    channel_t channel = (channel_t) chan;
    int data1 = 100;
    int data2 = 200;
    printf("协程 E 准备发送第一个数据: %d", data1);
    print_time();
    send_channel(channel, &data1);
    printf("协程 E 第一个数据发送完成");
    print_time();

    printf("协程 E 准备发送第二个数据: %d", data2);
    print_time();
    send_channel(channel, &data2);
    printf("协程 E 第二个数据发送完成");
    print_time();
}

void coF(void* chan)
{
    channel_t channel = (channel_t) chan;
    int recv_data;
    printf("协程 F 准备接收数据");
    print_time();
    recv_channel(channel, &recv_data);
    printf("协程 F 接收完成,数据: %d", recv_data);
    print_time();
}

void coG(void* chan)
{
    channel_t channel = (channel_t) chan;
    int data = 300;
    printf("协程 G 准备发送数据: %d", data);
    print_time();
    send_channel(channel, &data);
    printf("协程 G 发送完成");
    print_time();
}

void test03()
{
    printf("-------test03 start------------\n");
    channel_t channel = create_channel(4, 3);
    add_task(coE, channel);
    add_task(coF, channel);
    add_task(coG, channel);
    green_run();
    printf("destroy 返回：%d\n", destroy_channel(channel));
    printf("-------test03 end------------\n");
}

/*
场景 4：有缓冲 channel - 发送填满缓冲
测试目的    验证有缓冲 channel 当缓冲已满时发送阻塞
操作步骤
1. 创建有缓冲 channel（cap=2，elem_size=4）；
2. 启动协程 H：连续发送3个数据
3. 启动协程 I：接收1个数据
预期结果
1. 协程 H 发送前2个数据不阻塞；
2. 协程 H 发送第3个数据时阻塞；
3. 协程 I 接收数据后，协程 H 解除阻塞；
4. 所有数据正确传递。
*/

void coH(void* chan)
{
    channel_t channel = (channel_t) chan;
    int data[3] = {100, 200, 300};
    for (int i = 0; i < 3; i++) {
        printf("协程 H 准备发送第%d个数据: %d", i+1, data[i]);
        print_time();
        send_channel(channel, &data[i]);
        printf("协程 H 第%d个数据发送完成", i+1);
        print_time();
    }
}

void coI(void* chan)
{
    channel_t channel = (channel_t) chan;
    int recv_data;

    printf("协程 I 准备接收数据");
    print_time();
    recv_channel(channel, &recv_data);
    printf("协程 I 接收完成,数据: %d", recv_data);
    print_time();
}

void test04()
{
    printf("-------test04 start------------\n");
    channel_t channel = create_channel(4, 2);
    add_task(coH, channel);
    add_task(coI, channel);
    green_run();
    printf("destroy 返回：%d\n", destroy_channel(channel));
    printf("-------test04 end------------\n");
}

/*
场景 5：有缓冲 channel - 接收空缓冲
测试目的    验证有缓冲 channel 当缓冲为空时接收阻塞
操作步骤
1. 创建有缓冲 channel（cap=2，elem_size=4）；
2. 启动协程 J：立即尝试接收数据
3. 启动协程 K：发送1个数据
预期结果
1. 协程 J 接收时阻塞；
2. 协程 K 发送数据后，协程 J 解除阻塞；
3. 数据正确传递。
*/

void coJ(void* chan)
{
    channel_t channel = (channel_t) chan;
    int recv_data;
    printf("协程 J 准备接收数据");
    print_time();
    recv_channel(channel, &recv_data);
    printf("协程 J 接收完成,数据: %d", recv_data);
    print_time();
}

void coK(void* chan)
{
    channel_t channel = (channel_t) chan;
    int data = 500;
    printf("协程 K 准备发送数据: %d", data);
    print_time();
    send_channel(channel, &data);
    printf("协程 K 发送完成");
    print_time();
}

void test05()
{
    printf("-------test05 start------------\n");
    channel_t channel = create_channel(4, 2);
    add_task(coJ, channel);
    add_task(coK, channel);
    green_run();
    printf("destroy 返回：%d\n", destroy_channel(channel));
    printf("-------test05 end------------\n");
}

/*
场景 6：多对一通信 - 多个发送者一个接收者
测试目的    验证多个协程可以同时向同一个 channel 发送数据
操作步骤
1. 创建无缓冲 channel（cap=0，elem_size=4）；
2. 启动协程 L、M、N：分别发送不同的数据
3. 启动协程 O：连续接收3次数据
预期结果
1. 三个发送协程依次阻塞；
2. 接收协程每接收一次，唤醒一个发送协程；
3. 所有数据正确接收。
*/

void coL(void* chan)
{
    channel_t channel = (channel_t) chan;
    int data = 10;
    printf("协程 L 准备发送数据: %d", data);
    print_time();
    send_channel(channel, &data);
    printf("协程 L 发送完成");
    print_time();
}

void coM(void* chan)
{
    channel_t channel = (channel_t) chan;
    int data = 20;
    printf("协程 M 准备发送数据: %d", data);
    print_time();
    send_channel(channel, &data);
    printf("协程 M 发送完成");
    print_time();
}

void coN(void* chan)
{
    channel_t channel = (channel_t) chan;
    int data = 30;
    printf("协程 N 准备发送数据: %d", data);
    print_time();
    send_channel(channel, &data);
    printf("协程 N 发送完成");
    print_time();
}

void coO(void* chan)
{
    channel_t channel = (channel_t) chan;
    int recv_data;
    for (int i = 0; i < 3; i++) {
        printf("协程 O 准备接收第%d个数据", i+1);
        print_time();
        recv_channel(channel, &recv_data);
        printf("协程 O 第%d次接收完成,数据: %d", i+1, recv_data);
        print_time();
    }
}

void test06()
{
    printf("-------test06 start------------\n");
    channel_t channel = create_channel(4, 0);
    add_task(coL, channel);
    add_task(coM, channel);
    add_task(coN, channel);
    add_task(coO, channel);
    green_run();
    printf("destroy 返回：%d\n", destroy_channel(channel));
    printf("-------test06 end------------\n");
}

/*
场景 7：一对多通信 - 一个发送者多个接收者
测试目的    验证多个协程可以同时从同一个 channel 接收数据
操作步骤
1. 创建有缓冲 channel（cap=3，elem_size=4）；
2. 启动协程 P：连续发送3个数据
3. 启动协程 Q、R、S：分别接收数据
预期结果
1. 协程 P 发送完所有数据；
2. 三个接收协程都成功收到数据（可能有阻塞等待）；
3. 数据正确分配（接收顺序可能与发送顺序一致）。
*/

void coP(void* chan)
{
    channel_t channel = (channel_t) chan;
    int data[3] = {1000, 2000, 3000};
    for (int i = 0; i < 3; i++) {
        printf("协程 P 准备发送第%d个数据: %d", i+1, data[i]);
        print_time();
        send_channel(channel, &data[i]);
        printf("协程 P 第%d个数据发送完成", i+1);
        print_time();
    }
}

void coQ(void* chan)
{
    channel_t channel = (channel_t) chan;
    int recv_data;
    printf("协程 Q 准备接收数据");
    print_time();
    recv_channel(channel, &recv_data);
    printf("协程 Q 接收完成,数据: %d", recv_data);
    print_time();
}

void coR(void* chan)
{
    channel_t channel = (channel_t) chan;
    int recv_data;
    printf("协程 R 准备接收数据");
    print_time();
    recv_channel(channel, &recv_data);
    printf("协程 R 接收完成,数据: %d", recv_data);
    print_time();
}

void coS(void* chan)
{
    channel_t channel = (channel_t) chan;
    int recv_data;
    printf("协程 S 准备接收数据");
    print_time();
    recv_channel(channel, &recv_data);
    printf("协程 S 接收完成,数据: %d", recv_data);
    print_time();
}

void test07()
{
    printf("-------test07 start------------\n");
    channel_t channel = create_channel(4, 3);
    add_task(coP, channel);
    add_task(coQ, channel);
    add_task(coR, channel);
    add_task(coS, channel);
    green_run();
    printf("destroy 返回：%d\n", destroy_channel(channel));
    printf("-------test07 end------------\n");
}

/*
场景 8：大数据传输测试
测试目的    验证 channel 可以正确处理较大数据
操作步骤
1. 创建有缓冲 channel（cap=2，elem_size=1024）；
2. 启动协程 T：发送两个大数据块
3. 启动协程 U：接收两个大数据块
预期结果
1. 数据完整传输；
2. 内存访问正确；
3. 无崩溃、无数据损坏。
*/

struct BigData {
    char data[1024];
};

void coT(void* chan)
{
    channel_t channel = (channel_t) chan;
    struct BigData data1, data2;

    // 填充测试数据
    for (int i = 0; i < 1024; i++) {
        data1.data[i] = 'A' + (i % 26);
        data2.data[i] = 'a' + (i % 26);
    }
    data1.data[1023] = '\0';
    data2.data[1023] = '\0';

    printf("协程 T 准备发送第一个大数据块");
    print_time();
    send_channel(channel, &data1);
    printf("协程 T 第一个大数据块发送完成");
    print_time();

    printf("协程 T 准备发送第二个大数据块");
    print_time();
    send_channel(channel, &data2);
    printf("协程 T 第二个大数据块发送完成");
    print_time();
}

void coU(void* chan)
{
    channel_t channel = (channel_t) chan;
    struct BigData recv_data1, recv_data2;

    printf("协程 U 准备接收第一个大数据块");
    print_time();
    recv_channel(channel, &recv_data1);
    printf("协程 U 第一个大数据块接收完成，首字符: %c，尾字符: %c",
           recv_data1.data[0], recv_data1.data[1022]);
    print_time();

    printf("协程 U 准备接收第二个大数据块");
    print_time();
    recv_channel(channel, &recv_data2);
    printf("协程 U 第二个大数据块接收完成，首字符: %c，尾字符: %c",
           recv_data2.data[0], recv_data2.data[1022]);
    print_time();
}

void test08()
{
    printf("-------test08 start------------\n");
    channel_t channel = create_channel(sizeof(struct BigData), 2);
    add_task(coT, channel);
    add_task(coU, channel);
    green_run();
    printf("destroy 返回：%d\n", destroy_channel(channel));
    printf("-------test08 end------------\n");
}

/*
场景 9：channel 销毁测试 - 非空channel销毁
测试目的    验证 destroy_channel 当 channel 非空时返回失败
操作步骤
1. 创建有缓冲 channel（cap=2，elem_size=4）；
2. 发送1个数据但不接收；
3. 尝试销毁 channel；
预期结果
1. destroy_channel 返回 0（失败）；
2. 程序不崩溃。
*/
// 创建一个协程发送数据但不接收
void temp_co(void* chan) {
    int data = 999;
    channel_t ch = (channel_t) chan;
    printf("临时协程发送数据: %d", data);
    print_time();
    send_channel(ch, &data);
    printf("临时协程发送完成");
    print_time();
}

// 清理：创建一个接收协程清空channel
void cleanup_co(void* chan) {
    channel_t ch = (channel_t) chan;
    int recv_data;
    recv_channel(ch, &recv_data);
    printf("清理协程接收数据: %d", recv_data);
}
void test09()
{
    printf("-------test09 start------------\n");
    channel_t channel = create_channel(4, 2);


    // 注意：这里需要调度器运行完发送协程
    add_task(temp_co, channel);
    green_run();

    // 现在channel中有数据，尝试销毁
    printf("尝试销毁非空channel...\n");
    int result = destroy_channel(channel);
    printf("destroy 返回：%d (预期为0，表示失败)\n", result);



    add_task(cleanup_co, channel);
    green_run();

    // 现在应该可以成功销毁
    result = destroy_channel(channel);
    printf("清理后destroy返回：%d (预期为1，表示成功)\n", result);
    printf("-------test09 end------------\n");
}

/*
场景 10：边界测试 - 大容量 channel
测试目的    验证 channel 可以处理较大容量
操作步骤
1. 创建大容量 channel（cap=1000，elem_size=4）；
2. 发送1000个数据填满缓冲；
3. 接收1000个数据；
预期结果
1. 所有发送接收操作成功；
2. 内存管理正确；
3. 无崩溃。
*/

void sender_1000(void* chan)
{
    channel_t channel = (channel_t) chan;
    for (int i = 0; i < 1000; i++) {
        int data = i;
        send_channel(channel, &data);
        if (i % 100 == 0) {
            printf("发送进度: %d/1000", i);
            print_time();
        }
    }
    printf("发送完成: 1000/1000");
    print_time();
}

void receiver_1000(void* chan)
{
    channel_t channel = (channel_t) chan;
    int total = 0;
    for (int i = 0; i < 1000; i++) {
        int data;
        recv_channel(channel, &data);
        total += data;
        if (i % 100 == 0) {
            printf("接收进度: %d/1000, 当前数据: %d", i, data);
            print_time();
        }
    }
    printf("接收完成: 1000/1000, 总和: %d (预期: 499500)", total);
    print_time();
}

void test10()
{
    printf("-------test10 start------------\n");
    channel_t channel = create_channel(4, 1000);
    add_task(sender_1000, channel);
    add_task(receiver_1000, channel);
    green_run();
    printf("destroy 返回：%d\n", destroy_channel(channel));
    printf("-------test10 end------------\n");
}

// 更新main函数以运行所有测试
int main()
{
    printf("初始化时间锚点\n");
    init_time_anchor();
    //创建调度器
    create_scheduler();

    test01();  // 无缓冲 - 先发后收
    test02();  // 无缓冲 - 先收后发
    test03();  // 有缓冲 - 发送不满缓冲
    test04();  // 有缓冲 - 发送填满缓冲
    test05();  // 有缓冲 - 接收空缓冲
    test06();  // 多对一通信
    test07();  // 一对多通信
    test08();  // 大数据传输测试
    test09();  // channel销毁测试
    test10();  // 大容量channel测试

    return 0;
}
