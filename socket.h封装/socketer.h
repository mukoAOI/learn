//
// Created by mukoaoi on 24-9-10.
//

#ifndef EPOLL_SOCKETER_H
#define EPOLL_SOCKETER_H


#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

class Socket {
public:
    Socket();
    ~Socket();

    bool create();
    bool bind(const std::string& address, int port);
    bool listen(int backlog = 5);
    bool accept(Socket& new_socket);
    bool connect(const std::string& address, int port);
    bool send(const std::string& message) const;
    int receive(std::string& message) const;
    void close();

private:
    int sockfd;
    struct sockaddr_in addr;
};


#endif //EPOLL_SOCKETER_H
