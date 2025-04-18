#include "WSAInitializer.hpp"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>

static constexpr int BUFFER_LEN = 512;

void receiver(SOCKET sock, std::atomic<bool>& running) {
    char buffer[BUFFER_LEN];
    while (running) {
        int n = recv(sock, buffer, BUFFER_LEN, 0);
        if (n > 0) {
            std::cout << std::string(buffer, n);
        } else {
            running = false;
        }
    }
}

int main() {
    try {
        WSAInitializer wsa;
        struct addrinfo hints{}, *res;
        hints.ai_family   = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        if (getaddrinfo("127.0.0.1", "5723", &hints, &res) != 0)
            throw std::runtime_error("getaddrinfo failed");

        SOCKET sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sock == INVALID_SOCKET)
            throw std::runtime_error("Socket creation failed");

        if (connect(sock, res->ai_addr, (int)res->ai_addrlen) != 0)
            throw std::runtime_error("Connection failed");
        freeaddrinfo(res);

        std::string username;
        std::cout << "Enter your name: ";
        std::getline(std::cin, username);
        send(sock, username.c_str(), (int)username.size(), 0);

        std::atomic<bool> running{true};
        std::thread recvThread(receiver, sock, std::ref(running));

        while (running) {
            std::string line;
            std::getline(std::cin, line);
            if (line == "exit") break;
            send(sock, line.c_str(), (int)line.size(), 0);
        }

        running = false;
        recvThread.join();
        closesocket(sock);
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}
