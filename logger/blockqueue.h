#ifndef BLOCKQUEUE_H
#define BLOCKQUEUE_H

#include <deque>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <cassert>

template<class T>
class BlockDeque {
public:
    explicit BlockDeque(size_t MaxCapacity = 1000);

    ~BlockDeque();

    void clear();

    bool empty();

    bool full();

    void close();

    size_t size();

    size_t capacity();

    T front();

    T back();

    void push_back(const T &item);

    void push_front(const T &item);

    bool pop_front(T &item);

    bool pop_back(T &item);

    // 使用std::chrono::duration的超时版本
    bool pop_front(T &item, std::chrono::milliseconds timeout);

    bool pop_back(T &item, std::chrono::milliseconds timeout);

    void flush();

private:
    /*
     * 1. 防止消费者永远阻塞
在使用阻塞队列时，消费者（如 pop_front 或 pop_back 操作）通常会等待队列中有数据。如果队列一直为空，并且没有数据被生产者放入，消费者将一直阻塞下去。超时时间通过设定一个最大等待时间，使消费者在超过这个时间后自动退出等待，从而防止无限期阻塞。

2. 在队列为空时控制等待时间
当队列为空时，消费者调用 pop_front 或 pop_back 操作会等待队列中有新数据。这种等待通常是通过条件变量（condition_variable）来实现的。超时时间决定了消费者最多等待多长时间：

如果在设定的超时时间内，队列有数据，则消费者成功获取数据。
如果超过了设定的超时时间队列仍然没有数据，消费者会触发超时，结束等待。
3. 处理队列关闭的情况
当队列被关闭时（比如通过调用 close 函数），生产者和消费者都需要知道队列已经不再工作。超时机制允许消费者在等待过程中及时检查队列的状态，确保在超时后能够退出。

4. 提升程序的灵活性
在某些场景下，可能不希望消费者无限期地等待数据，而是希望在超时后进行一些其他操作，比如重新请求数据、记录日志、或者决定终止线程。设定超时时间可以为程序提供更好的控制和灵活性。
     *
     */
    bool wait_for_consumer(std::unique_lock<std::mutex>& locker, std::chrono::milliseconds timeout);

    bool wait_for_producer(std::unique_lock<std::mutex>& locker, std::chrono::milliseconds timeout);

private:
    std::deque<T> deque_;
    size_t capacity_;
    std::mutex mtx_;
    bool isClosed_;
    std::condition_variable condConsumer_;
    std::condition_variable condProducer_;
};

template<class T>
BlockDeque<T>::BlockDeque(size_t MaxCapacity) : capacity_(MaxCapacity), isClosed_(false) {
    assert(MaxCapacity > 0);
}

template<class T>
BlockDeque<T>::~BlockDeque() {
    close();
}

template<class T>
void BlockDeque<T>::close() {
    {
        std::lock_guard<std::mutex> locker(mtx_);
        isClosed_ = true;
        deque_.clear();
    }
    condProducer_.notify_all();
    condConsumer_.notify_all();
}

template<class T>
void BlockDeque<T>::clear() {
    std::lock_guard<std::mutex> locker(mtx_);
    deque_.clear();
}

template<class T>
bool BlockDeque<T>::empty() {
    std::lock_guard<std::mutex> locker(mtx_);
    return deque_.empty();
}

template<class T>
void BlockDeque<T>::flush() {
    condConsumer_.notify_one();
}

template<class T>
bool BlockDeque<T>::full() {
    std::lock_guard<std::mutex> locker(mtx_);
    return deque_.size() >= capacity_;
}



template<class T>
size_t BlockDeque<T>::size() {
    std::lock_guard<std::mutex> locker(mtx_);
    return deque_.size();
}

template<class T>
size_t BlockDeque<T>::capacity() {
    return capacity_;
}

template<class T>
T BlockDeque<T>::front() {
    std::lock_guard<std::mutex> locker(mtx_);
    return deque_.front();
}

template<class T>
T BlockDeque<T>::back() {
    std::lock_guard<std::mutex> locker(mtx_);
    return deque_.back();
}

template<class T>
void BlockDeque<T>::push_back(const T &item) {
    std::unique_lock<std::mutex> locker(mtx_);
    while (deque_.size() >= capacity_) {
        condProducer_.wait(locker);
        if (isClosed_) return;
    }
    deque_.push_back(item);
    condConsumer_.notify_one();
}

template<class T>
void BlockDeque<T>::push_front(const T &item) {
    std::unique_lock<std::mutex> locker(mtx_);
    while (deque_.size() >= capacity_) {
        condProducer_.wait(locker);
        if (isClosed_) return;
    }
    deque_.push_front(item);
    condConsumer_.notify_one();
}

template<class T>
bool BlockDeque<T>::pop_front(T &item) {
    std::unique_lock<std::mutex> locker(mtx_);
    while (deque_.empty()) {
        condConsumer_.wait(locker);
        if (isClosed_) return false;
    }
    item = deque_.front();
    deque_.pop_front();
    condProducer_.notify_one();
    return true;
}

template<class T>
bool BlockDeque<T>::pop_back(T &item) {
    std::unique_lock<std::mutex> locker(mtx_);
    while (deque_.empty()) {
        condConsumer_.wait(locker);
        if (isClosed_) return false;
    }
    item = deque_.back();
    deque_.pop_back();
    condProducer_.notify_one();
    return true;
}

// 修改后的超时版本，使用std::chrono::milliseconds作为超时时间
template<class T>
bool BlockDeque<T>::pop_front(T &item, std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> locker(mtx_);
    if (!wait_for_consumer(locker, timeout)) {
        return false;
    }
    item = deque_.front();
    deque_.pop_front();
    condProducer_.notify_one();
    return true;
}

template<class T>
bool BlockDeque<T>::pop_back(T &item, std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> locker(mtx_);
    if (!wait_for_consumer(locker, timeout)) {
        return false;
    }
    item = deque_.back();
    deque_.pop_back();
    condProducer_.notify_one();
    return true;
}

template<class T>
bool BlockDeque<T>::wait_for_consumer(std::unique_lock<std::mutex>& locker, std::chrono::milliseconds timeout) {
    if (deque_.empty()) {
        if (condConsumer_.wait_for(locker, timeout) == std::cv_status::timeout) {
            return false;
        }
        if (isClosed_) return false;
    }
    return true;
    return true;
}

template<class T>
bool BlockDeque<T>::wait_for_producer(std::unique_lock<std::mutex>& locker, std::chrono::milliseconds timeout) {
    if (deque_.size() >= capacity_) {
        if (condProducer_.wait_for(locker, timeout) == std::cv_status::timeout) {
            return false;
        }
        if (isClosed_) return false;
    }
    return true;
}

#endif // BLOCKQUEUE_H
