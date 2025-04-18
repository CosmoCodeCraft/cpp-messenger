#include "WSAInitializer.hpp"
#include "Server.hpp"
#include <ws2tcpip.h>
#include <iostream>
#include <sstream>

static constexpr int MAX_CLIENTS = 100;
static constexpr int BUFFER_LEN = 512;

Server::Server(unsigned short port) {
    WSAInitializer wsa;
    struct addrinfo hints{}, *res;
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags    = AI_PASSIVE;

    auto portStr = std::to_string(port);
    if (getaddrinfo(NULL, portStr.c_str(), &hints, &res) != 0)
        throw std::runtime_error("getaddrinfo failed");

    listenSocket_ = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (listenSocket_ == INVALID_SOCKET)
        throw std::runtime_error("Socket creation failed");

    if (bind(listenSocket_, res->ai_addr, (int)res->ai_addrlen) == SOCKET_ERROR)
        throw std::runtime_error("Bind failed");

    freeaddrinfo(res);

    if (listen(listenSocket_, SOMAXCONN) == SOCKET_ERROR)
        throw std::runtime_error("Listen failed");

    clientActive_.assign(MAX_CLIENTS, false);
}

Server::~Server() {
    running_ = false;
    if (listenSocket_ != INVALID_SOCKET) {
        closesocket(listenSocket_);
    }
}

void Server::run() {
    running_ = true;
    std::thread(&Server::acceptLoop, this).detach();
    std::cout << "Server is running..." << std::endl;
    // Block main thread until shutdown
    while (running_) std::this_thread::sleep_for(std::chrono::seconds(1));
}

void Server::acceptLoop() {
    while (running_) {
        SOCKET clientSock = accept(listenSocket_, nullptr, nullptr);
        if (clientSock == INVALID_SOCKET) continue;
        int id = getNextClientId();
        if (id < 0) {
            const auto msg = "Server full\n";
            send(clientSock, msg, (int)strlen(msg), 0);
            closesocket(clientSock);
            continue;
        }
        {
            std::lock_guard lock(mutex_);
            clientActive_[id] = true;
            clientSockets_.push_back(clientSock);
        }
        std::thread(&Server::handleClient, this, clientSock, id).detach();
    }
}

int Server::getNextClientId() {
    std::lock_guard lock(mutex_);
    for (int i = 0; i < MAX_CLIENTS; ++i)
        if (!clientActive_[i]) return i;
    return -1;
}

void Server::broadcast(const std::string& text, SOCKET exclude) {
    std::lock_guard lock(mutex_);
    for (auto s : clientSockets_) {
        if (s != exclude) send(s, text.c_str(), (int)text.size(), 0);
    }
}

void Server::handleClient(SOCKET sock, int clientId) {
    char buffer[BUFFER_LEN]{};
    std::string username;

    // Receive first message as username
    int bytes = recv(sock, buffer, BUFFER_LEN, 0);
    if (bytes <= 0) return;
    username.assign(buffer, bytes);
    std::ostringstream connMsg;
    connMsg << "[Server] " << username << " joined (ID:" << clientId << ")\n";
    broadcast(connMsg.str(), sock);

    while ((bytes = recv(sock, buffer, BUFFER_LEN, 0)) > 0) {
        std::string msg(buffer, bytes);
        // Simple protocol: "toID: message"
        auto sep = msg.find(':');
        if (sep != std::string::npos) {
            int toId = std::stoi(msg.substr(0, sep));
            std::string content = msg.substr(sep + 1);
            std::lock_guard lock(mutex_);
            if (toId >=0 && toId < (int)clientSockets_.size() && clientActive_[toId]) {
                SOCKET toSock = clientSockets_[toId];
                std::ostringstream out;
                out << username << ": " << content << '\n';
                send(toSock, out.str().c_str(), (int)out.str().size(), 0);
            }
        }
    }

    // Cleanup
    closesocket(sock);
    {
        std::lock_guard lock(mutex_);
        clientActive_[clientId] = false;
        clientSockets_.erase(std::remove(clientSockets_.begin(), clientSockets_.end(), sock), clientSockets_.end());
    }
    std::ostringstream discMsg;
    discMsg << "[Server] " << username << " disconnected (ID:" << clientId << ")\n";
    broadcast(discMsg.str());
}
