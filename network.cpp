#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include "network.h"

#define MAXLEN 1024

Network::Network() {}

int Network::createUDPSocket() {
    int sock_udp = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_udp < 0 ) {
        std::cout<<"UDP socket creation failed"<<std::endl;
		exit(EXIT_FAILURE);
	}
    return sock_udp;
}

int Network::bindUDPSocket(int sock_udp, sockaddr_in addr) {
    int n = bind(sock_udp, (const struct sockaddr *)&addr, sizeof(addr));
    if (n < 0 ) {
        std::cout<<"UDP socket bind failed"<<std::endl;
		exit(EXIT_FAILURE);
	}
    return n;
}

int Network::sendToUDPSocket(int sock_udp, const char * resp, sockaddr_in addr) {
    socklen_t len = sizeof(addr);
    int n = sendto(sock_udp, (const char *)resp, strlen(resp), 0, 
                (const struct sockaddr *) &addr,len);
    return n;
}

int Network::recvFromUDPSocket(int sock_udp, sockaddr_in &addr, char * buffer) {
    socklen_t len = sizeof(addr);
    int n = recvfrom(sock_udp, (char *)buffer, MAXLEN,
				MSG_WAITALL, (struct sockaddr *) &addr, &len);
    return n;
}

// Get free port number by protocols(TCP/UDP).
int Network::getFreePort(int protocol) {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = 0;

    socklen_t socklen = sizeof(addr);

    int sock_fd = socket(AF_INET, protocol, 0);
    if (sock_fd < 0) {
        std::cout<<"Socket creation failed."<<std::endl;
        exit(EXIT_FAILURE);
    }

    if (bind(sock_fd, (struct sockaddr *)&addr, socklen)) {
        std::cout<<"Socket bind failed."<<std::endl;
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    getsockname(sock_fd, (struct sockaddr *)&addr, &socklen);
    
    return addr.sin_port;
}

int Network::createTCPSocket() {
    int sock_tcp = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_tcp < 0 ) {
        std::cout<<"TCP socket creation failed."<<std::endl;
        exit(EXIT_FAILURE);
    }
    return sock_tcp;
}

// TCP connect
int Network::initiateTCPConnection(in_addr_t toAddr, int toPort, int sock_tcp) {
    struct sockaddr_in clientAddr;
    memset(&clientAddr, 0, sizeof(clientAddr));
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(toPort);
    clientAddr.sin_addr.s_addr = toAddr;

    if (connect(sock_tcp, (struct sockaddr *)&clientAddr, sizeof(clientAddr)) < 0) {
        std::cout<<"TCP socket connect failed."<<std::endl;
        close(sock_tcp);
        exit(EXIT_FAILURE);
    }
    
    return sock_tcp;
}

// TCP bind, listen and accept
int Network::acceptTCPConnection(int port, int sock_tcp) {
    struct sockaddr_in serverAddr, clientAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    memset(&clientAddr, 0, sizeof(clientAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if(bind(sock_tcp, (struct sockaddr* ) &serverAddr, sizeof(serverAddr)) < 0) {
        std::cout<<"TCP socket bind error."<<std::endl;
        close(sock_tcp);
        exit(EXIT_FAILURE);
    }

    if(listen(sock_tcp, 10) < 0 ) {
        std::cout<<"TCP socket listen error."<<std::endl;
        close(sock_tcp);
        exit(EXIT_FAILURE);
    }

    socklen_t length = sizeof(clientAddr);
    int conn = accept(sock_tcp, (struct sockaddr*) &clientAddr, &length);

    if (conn < 0) {
        std::cout<<"TCP socket accept error."<<std::endl;
        close(sock_tcp);
        exit(EXIT_FAILURE);
    }

    return conn;
}

// Sendfile to socket.
int Network::sendFile(const char* fileName, int sock_fd) {
    FILE* fp = fopen(fileName, "rb");
    if (fp == NULL) {
        std::cout<<"File does not exist."<<std::endl;
        exit(EXIT_FAILURE);
    }

    char Buffer[MAXLEN];
    unsigned long long size = 0;
    unsigned long long total = 0;

    while ((size = fread(Buffer, sizeof(char), MAXLEN, fp))) {
        if (send(sock_fd, Buffer, size, 0) < 0) {
            std::cout<<"TCP socket send error."<<std::endl;
        }
        total += size;
        memset(&Buffer, 0, MAXLEN); // Empty the buffer.
    }

    std::cout<<"File sent, size: "<<total<<std::endl;

    return total; // todoss
}

// Receive file from socket and write local file.
int Network::receiveFile(const char* fileName, int sock_fd) {
    char Buffer[MAXLEN];
    memset(&Buffer, 0, MAXLEN);
    unsigned long long size = 0;

    size = recv(sock_fd, Buffer, MAXLEN, 0);

    std::cout<<"File received, size is: "<<size<<std::endl;

    FILE* fp = fopen(fileName, "wb");
    // Max file size is 1024.
    if (fwrite(Buffer, sizeof(char), size, fp) < size) {
        std::cout<<"File write error."<<std::endl;
        exit(EXIT_FAILURE);
    }
    
    return size;
}

bool Network::checkIPAddr(char* ip) {
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ip, &(sa.sin_addr));
    return result != 0;
}

bool isNumeric(std::string const &str) {
    auto it = str.begin();
    while (it != str.end() && std::isdigit(*it)) {
        it++;
    }
    return !str.empty() && it == str.end();
}


bool Network::checkPort(char* port) {
    std::string portStr = (std::string) port;
    int portNum = stoi(portStr);

    if (!isNumeric(portStr)) {
        return false;
    }

    if (portNum >= 1 && portNum <= 65535) {
        return true;
    }

    return false;
}


bool Network::checkFileExist(char * fileName) {
    if (FILE *file = fopen(fileName, "r")) {
        return true;
    }
    return false;
}