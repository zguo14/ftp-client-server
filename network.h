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
    int createTCPSocket();
    int initiateTCPConnection(in_addr_t toAddr, int toPort, int sock_tcp);
    int acceptTCPConnection(int port, int sock_tcp);
    int sendFile(const char* fileName, int sock_fd);
    int receiveFile(const char* fileName, int sock_fd);

    bool checkIPAddr(char* ip);
    bool checkPort(char* port);
};

#endif