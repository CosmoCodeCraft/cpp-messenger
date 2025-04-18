#pragma once
#include <winsock2.h>
#include <stdexcept>

class WSAInitializer {
public:
    WSAInitializer() {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            throw std::runtime_error("WSAStartup failed");
        }
    }

    ~WSAInitializer() {
        WSACleanup();
    }
};
