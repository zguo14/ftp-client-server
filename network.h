#ifndef NETWORK_H
#define NETWORK_H
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

class Network
{
  public:
    Network();
    int getFreePort(int protocol);
    int createUDPSocket();
    int bindUDPSocket(int sock_udp, sockaddr_in addr);
    int sendToUDPSocket(int sock_udp, const char * resp, sockaddr_in addr);
    int recvFromUDPSocket(int sock_udp, sockaddr_in &addr, char * buffer);
    int createTCPSocket();
    int initiateTCPConnection(in_addr_t toAddr, int toPort, int sock_tcp);
    int acceptTCPConnection(int port, int sock_tcp);
    int sendFile(const char* fileName, int sock_fd);
    int receiveFile(const char* fileName, int sock_fd);

    bool checkIPAddr(char* ip);
    bool checkPort(char* port);
    bool checkFileExist(char * fileName);
};

#endif