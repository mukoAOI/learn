#include <iostream>
#include <string>
#include <cstring>
#include <cassert>
#include <unistd.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

#define MAX_EVENTS_NUMBER 5

class EpollServer {
public:
    EpollServer(const std::string& ip, int port)
            : ip_(ip), port_(port), listenfd_(-1), epollfd_(-1) {}

    ~EpollServer() {
        if (listenfd_ != -1) close(listenfd_);
        if (epollfd_ != -1) close(epollfd_);
    }

    void run() {
        setupServer();
        eventLoop();
    }

private:
    std::string ip_;
    int port_;
    int listenfd_;
    int epollfd_;

    void setupServer() {
        listenfd_ = socket(PF_INET, SOCK_STREAM, 0);
        assert(listenfd_ >= 1);

        struct sockaddr_in address;
        std::memset(&address, 0, sizeof(address));
        address.sin_family = AF_INET;
        address.sin_port = htons(port_);
        inet_pton(AF_INET, ip_.c_str(), &address.sin_addr);

        int ret = bind(listenfd_, reinterpret_cast<struct sockaddr*>(&address), sizeof(address));
        assert(ret != -1);

        ret = listen(listenfd_, 5);
        assert(ret != -1);

        epollfd_ = epoll_create(5);
        assert(epollfd_ != -1);
        addfd(epollfd_, listenfd_);
    }

    void setNonBlocking(int fd) {
        int old_state = fcntl(fd, F_GETFL);
        int new_state = old_state | O_NONBLOCK;
        fcntl(fd, F_SETFL, new_state);
    }

    void addfd(int epollfd, int fd) {
        epoll_event event;
        event.events = EPOLLIN | EPOLLET;
        event.data.fd = fd;
        epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
        setNonBlocking(fd);
    }

    void handleConnection() {
        struct sockaddr_in client;
        socklen_t client_addrlength = sizeof(client);
        int sockfd = accept(listenfd_, reinterpret_cast<struct sockaddr*>(&client), &client_addrlength);
        addfd(epollfd_, sockfd);
    }

    void handleEvent(int eventfd) {
        char buf[1024] = {0};
        while (true) {
            std::memset(buf, '\0', sizeof(buf));
            int recv_size = recv(eventfd, buf, sizeof(buf), 0);
            if (recv_size < 0) {
                if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                    break;
                }
                std::cerr << "sockfd " << eventfd << ", recv msg failed\n";
                close(eventfd);
                break;
            } else if (recv_size == 0) {
                close(eventfd);
                break;
            } else {
                send(eventfd, buf, recv_size, 0);
            }
        }
    }

    void eventLoop() {
        epoll_event events[MAX_EVENTS_NUMBER];

        while (true) {
            int number = epoll_wait(epollfd_, events, MAX_EVENTS_NUMBER, -1);
            if (number < 0) {
                std::cerr << "epoll_wait failed\n";
                break;
            }

            for (int i = 0; i < number; ++i) {
                const auto& event = events[i];
                const auto eventfd = event.data.fd;

                if (eventfd == listenfd_) {
                    handleConnection();
                } else if (event.events & EPOLLIN) {
                    handleEvent(eventfd);
                }
            }
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc <= 2) {
        std::cout << "Usage: " << argv[0] << " ip_address portname\n";
        return 0;
    }

    std::string ip = argv[1];
    int port = std::stoi(argv[2]);

    EpollServer server(ip, port);
    server.run();

    return 0;
}
