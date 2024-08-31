//
// Created by 23785 on 24-8-31.
//

#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class EchoClient {
public:
    EchoClient(const std::string& ip, int port)
            : ip_(ip), port_(port), sockfd_(-1) {}

    ~EchoClient() {
        if (sockfd_ != -1) {
            close(sockfd_);
        }
    }

    bool connectToServer() {
        sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd_ < 0) {
            std::cerr << "Failed to create socket.\n";
            return false;
        }

        struct sockaddr_in server_addr;
        std::memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port_);
        inet_pton(AF_INET, ip_.c_str(), &server_addr.sin_addr);

        if (connect(sockfd_, reinterpret_cast<struct sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
            std::cerr << "Connection failed.\n";
            return false;
        }

        std::cout << "Connected to server.\n";
        return true;
    }

    void echo() {
        std::string message;
        char buffer[1024];

        while (true) {
            std::cout << "Enter message: ";
            std::getline(std::cin, message);

            if (message == "exit") {
                break;
            }

            send(sockfd_, message.c_str(), message.size(), 0);

            std::memset(buffer, 0, sizeof(buffer));
            int recv_size = recv(sockfd_, buffer, sizeof(buffer) - 1, 0);

            if (recv_size > 0) {
                std::cout << "Received: " << buffer << "\n";
            } else if (recv_size == 0) {
                std::cout << "Server closed the connection.\n";
                break;
            } else {
                std::cerr << "Receive failed.\n";
                break;
            }
        }
    }

private:
    std::string ip_;
    int port_;
    int sockfd_;
};

int main(int argc, char* argv[]) {
    if (argc <= 2) {
        std::cout << "Usage: " << argv[0] << " ip_address portname\n";
        return 0;
    }

    std::string ip = argv[1];
    int port = std::stoi(argv[2]);

    EchoClient client(ip, port);
    if (client.connectToServer()) {
        client.echo();
    }

    return 0;
}

