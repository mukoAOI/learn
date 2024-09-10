//
// Created by mukoaoi on 24-9-10.
//

#include "socketer.h"


Socket::Socket() : sockfd(-1) {
    memset(&addr, 0, sizeof(addr));
}

Socket::~Socket() {
    close();
}

bool Socket::create() {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    return sockfd != -1;
}

bool Socket::bind(const std::string& address, int port) {
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(address.c_str());

    return ::bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) != -1;
}

bool Socket::listen(int backlog) {
    return ::listen(sockfd, backlog) != -1; //使用了:: 来强制调用全局的 listen 函数。
}

bool Socket::accept(Socket& new_socket) {
    int addr_len = sizeof(addr);
    new_socket.sockfd = ::accept(sockfd, (struct sockaddr*)&addr, (socklen_t*)&addr_len);
    return new_socket.sockfd != -1;
}

bool Socket::connect(const std::string& address, int port) {
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(address.c_str());

    return ::connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) != -1;
}

bool Socket::send(const std::string& message) const {
    return ::send(sockfd, message.c_str(), message.size(), 0) != -1;
}

int Socket::receive(std::string& message) const {
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    int bytes_received = ::recv(sockfd, buffer, sizeof(buffer), 0);

    if (bytes_received > 0) {
        message.assign(buffer, bytes_received);
    }

    return bytes_received;
}

void Socket::close() {
    if (sockfd != -1) {
        ::close(sockfd);
        sockfd = -1;
    }
}
