#include "buffer.h"
#include <iostream>
#include <cassert>
using  namespace std;
void TestBuffer() {
    std::cout << "Testing Buffer class...\n";

    Buffer buf;

    // 测试追加数据
    buf.Append("Hello, World!", 13);
    assert(buf.ReadableBytes() == 13);
    assert(std::string(buf.Peek(), buf.ReadableBytes()) == "Hello, World!");

    // 测试读取数据
    buf.Retrieve(5);
    assert(buf.ReadableBytes() == 8);
    assert(std::string(buf.Peek(), buf.ReadableBytes()) == ", World!");

    // 测试追加更多数据
    const char *str=" C++ Buffer";
    buf.Append(str, strlen(str));
    assert(buf.ReadableBytes() == strlen(str)+8);
    assert(std::string(buf.Peek(), buf.ReadableBytes()) == ", World! C++ Buffer");

    // 测试 RetrieveUntil
    const char* end = buf.Peek() + 8;  // 指向 " C++ Buffer"
    buf.RetrieveUntil(end);
    assert(buf.ReadableBytes() == 11);
    assert(std::string(buf.Peek(), buf.ReadableBytes()) == " C++ Buffer");

    // 测试 RetrieveAllToStr
    std::string content = buf.RetrieveAllToStr();
    assert(content == " C++ Buffer");
    assert(buf.ReadableBytes() == 0);

    // 测试写入文件描述符
    int fd = 1;  // 标准输出
    int errno;
    buf.Append("Testing write to file descriptor.", 34);
    ssize_t written = buf.WriteFd(fd, &errno);
    assert(written == 34);

    std::cout << "Buffer content after WriteFd: " << std::string(buf.Peek(), buf.ReadableBytes()) << "\n";
    assert(buf.ReadableBytes() == 0);

    // 测试线程安全的 TryRetrieve
    buf.Append("Thread safety test.", 19);
    bool result = buf.TryRetrieve(10);
    assert(result);
    assert(buf.ReadableBytes() == 9);
    std::cout << "Buffer content after TryRetrieve: " << std::string(buf.Peek(), buf.ReadableBytes()) << "\n";

    std::cout << "All tests passed!\n";
}

int main() {
    TestBuffer();
    return 0;
}
