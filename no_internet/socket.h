#pragma once

#include <string>
#include <stdexcept>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

class Socket {
public:
    Socket();
    ~Socket();
    bool connect(const std::string& host, int port);
    int send(const std::string& data);
    int receive(std::string& buffer, int maxLen = 1024);
    void close();
    bool isValid() const;
private:
    SOCKET sockfd;
    bool valid;
};
