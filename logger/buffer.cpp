#include "Buffer.h"

Buffer::Buffer(int initBuffSize)
        : buffer_(initBuffSize),
          readPos_(0),
          writePos_(0) {
}

size_t Buffer::WritableBytes() const {
    return buffer_.size() - writePos_.load();
}

size_t Buffer::ReadableBytes() const {
    return writePos_.load() - readPos_.load();
}

size_t Buffer::PrependableBytes() const {
    return readPos_.load();
}

const char* Buffer::Peek() const {
    return BeginPtr_() + readPos_.load();
}

void Buffer::EnsureWriteable(size_t len) {
    if (WritableBytes() < len) {
        MakeSpace_(len);
    }
}

void Buffer::HasWritten(size_t len) {
    writePos_.fetch_add(len);
}

void Buffer::Retrieve(size_t len) {
    assert(len <= ReadableBytes());
    readPos_.fetch_add(len);
}

void Buffer::RetrieveUntil(const char* end) {
    assert(Peek() <= end && end <= BeginPtr_() + writePos_.load());
    Retrieve(end - Peek());
}

void Buffer::RetrieveAll() {
    Retrieve(ReadableBytes());
}

std::string Buffer::RetrieveAllToStr() {
    std::string result(Peek(), ReadableBytes());
    RetrieveAll();
    return result;
}

const char* Buffer::BeginWriteConst() const {
    return BeginPtr_() + writePos_.load();
}

char* Buffer::BeginWrite() {
    return BeginPtr_() + writePos_.load();
}

void Buffer::Append(const std::string& str) {
    Append(str.data(), str.size());
}

void Buffer::Append(const char* str, size_t len) {
    EnsureWriteable(len);
    std::copy(str, str + len, BeginWrite());
    HasWritten(len);
}

void Buffer::Append(const void* data, size_t len) {
    Append(static_cast<const char*>(data), len);
}

void Buffer::Append(const Buffer& buff) {
    Append(buff.Peek(), buff.ReadableBytes());
}

ssize_t Buffer::ReadFd(int fd, int* Errno) {
    char buf[65536];
    ssize_t n = read(fd, buf, sizeof(buf));
    if (n > 0) {
        Append(buf, n);
    } else if (n < 0) {
        *Errno = errno;
    }
    return n;
}

ssize_t Buffer::WriteFd(int fd, int* Errno) {
    ssize_t n = write(fd, Peek(), ReadableBytes());
    if (n > 0) {
        Retrieve(n);
    } else if (n < 0) {
        *Errno = errno;
    }
    return n;
}

bool Buffer::TryRetrieve(size_t len) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (ReadableBytes() >= len) {
        Retrieve(len);
        return true;
    }
    return false;
}

char* Buffer::BeginPtr_() {
    return buffer_.data();
}

const char* Buffer::BeginPtr_() const {
    return buffer_.data();
}

void Buffer::MakeSpace_(size_t len) {
    if (WritableBytes() + PrependableBytes() < len) {
        // 扩展缓冲区
        size_t readable = ReadableBytes();
        buffer_.resize(writePos_.load() + len);
        // 重新定位读写位置
        readPos_.store(0);
        writePos_.store(readable);
    } else {
        // 移动数据到缓冲区开头
        size_t readable = ReadableBytes();
        std::copy(BeginPtr_() + readPos_.load(), BeginPtr_() + writePos_.load(), BeginPtr_());
        readPos_.store(0);
        writePos_.store(readable);
    }
}

