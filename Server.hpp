#pragma once

#include <winsock2.h>
#include <vector>
#include <string>
#include <mutex>
#include <thread>
#include <atomic>

class Server {
public:
    Server(unsigned short port = 5723);
    ~Server();
    void run();

private:
    void acceptLoop();
    void handleClient(SOCKET clientSocket, int clientId);
    int getNextClientId();
    void broadcast(const std::string& message, SOCKET excludeSocket = INVALID_SOCKET);

    SOCKET listenSocket_;
    std::atomic<bool> running_;
    std::vector<SOCKET> clientSockets_;
    std::vector<bool> clientActive_;
    std::mutex mutex_;
};
