#include "log.h"
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>

// 每个线程写入的日志消息
void log_messages(int thread_id) {
    for (int i = 0; i < 20; ++i) {
        LOG_DEBUG("Thread %d - Debug message %d", thread_id, i);
        LOG_INFO("Thread %d - Info message %d", thread_id, i);
        LOG_WARN("Thread %d - Warning message %d", thread_id, i);
        LOG_ERROR("Thread %d - Error message %d", thread_id, i);

        // 让每个线程在写入日志后稍微休息一下
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void multi_thread_log_test() {
    // 初始化日志系统
    Log::get_log()->init(2, "./test_log", ".log", 1024);

    // 创建多个线程来写入日志
    const int num_threads = 4; // 线程数量
    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(log_messages, i);
    }

    // 等待所有线程完成
    for (auto& th : threads) {
        if (th.joinable()) {
            th.join();
        }
    }

    // 等待异步写线程完成写入
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // 清理日志系统
    Log::get_log()->flush();
}

int main() {
    multi_thread_log_test();
    return 0;
}
