#include "socket.h"
#include <cstring>

Socket::Socket() : valid(false) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        throw std::runtime_error("WSAStartup failed");
    }
    sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET) {
        WSACleanup();
        throw std::runtime_error("Socket creation failed");
    }
    valid = true;
}

Socket::~Socket() {
    close();
    WSACleanup();
}

bool Socket::connect(const std::string& host, int port) {
    if (!valid) throw std::runtime_error("Invalid socket");
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(host.c_str());
    int res = ::connect(sockfd, (sockaddr*)&addr, sizeof(addr));
    if (res != 0) {
        throw std::runtime_error("Connect failed");
    }
    return true;
}

int Socket::send(const std::string& data) {
    if (!valid) throw std::runtime_error("Invalid socket");
    int sent = ::send(sockfd, data.c_str(), static_cast<int>(data.size()), 0);
    if (sent == SOCKET_ERROR) {
        throw std::runtime_error("Send failed");
    }
    return sent;
}

int Socket::receive(std::string& buffer, int maxLen) {
    if (!valid) throw std::runtime_error("Invalid socket");
    char* temp = new char[maxLen];
    int received = ::recv(sockfd, temp, maxLen, 0);
    if (received == SOCKET_ERROR) {
        delete[] temp;
        throw std::runtime_error("Receive failed");
    }
    if (received > 0) {
        buffer.assign(temp, received);
    }
    delete[] temp;
    return received;
}

void Socket::close() {
    if (valid) {
        closesocket(sockfd);
        valid = false;
    }
}

bool Socket::isValid() const {
    return valid;
}
