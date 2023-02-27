#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include "network.h"

#define MAXLINE 1024

int main(int argc, char *argv[]) {
    if (argc != 6) {
        std::cout<<"Invalid params. Usage: ./client.sh <server address> <n_port>" 
        "<mode> <req_code> \'received.txt\'"<<std::endl;
        exit(EXIT_FAILURE);
    }

    char* serverAddr = argv[1];
    char* serverPort = argv[2];
    char* mode = argv[3];
    char* reqCode = argv[4];
    char* fileName = argv[5];

    int sock_udp;
    char buffer[MAXLINE];

    struct sockaddr_in	 servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(serverPort));
    servaddr.sin_addr.s_addr = inet_addr(serverAddr);

    Network *network = new Network();

    // Input command check.
    if (!network->checkIPAddr(serverAddr)) {
        std::cout<<"Invalid IP address."<<std::endl;
		exit(EXIT_FAILURE);
    }

    if (!network->checkPort(serverPort)) {
        std::cout<<"Invalid port number."<<std::endl;
		exit(EXIT_FAILURE);
    }

    std::string modeStr = (std::string)mode;
    if (!(modeStr == "PORT" || modeStr == "PASV")) {
        std::cout<<"Invalid mode."<<std::endl;
		exit(EXIT_FAILURE);
    }

    // Create UDP socket
    sock_udp = network->createUDPSocket();
	
    // Active mode.
    if (strcmp(mode, "PORT") == 0) {
        socklen_t len;
        const char *request;
    
        int port = network->getFreePort(SOCK_STREAM);
        std::string cmd = "PORT " + std::to_string(port) + " " + (std::string)reqCode + " ";
        request = cmd.c_str();
        
        // Negotiation using UDP.
        network->sendToUDPSocket(sock_udp, request, servaddr);
        network->recvFromUDPSocket(sock_udp, servaddr, buffer);

        if (buffer[0] == '0') {
            std::cout<<"Wrong req_code."<<std::endl;
            exit(EXIT_FAILURE);
        }

        // Create TCP socket and wait file transfer from server.
        int sock_tcp = network->createTCPSocket();
        int sock_new = network->acceptTCPConnection(port, sock_tcp);
        network->receiveFile(fileName, sock_new);

        close(sock_new);
        close(sock_tcp);
    }

    // Passive mode.
    if (strcmp(mode, "PASV") == 0) {
        socklen_t len;
        const char *request;
    
        std::string cmd = "PASV " + (std::string)reqCode + " ";
        request = cmd.c_str();

        // Negotiation using UDP.
        network->sendToUDPSocket(sock_udp, request, servaddr);
        network->recvFromUDPSocket(sock_udp, servaddr, buffer);

        if (buffer[0] == '0') {
            std::cout<<"Wrong req_code."<<std::endl;
            exit(EXIT_FAILURE);
        }
        
        // Extract r_port sent by server.
        int i = 0;
        std::string tmp = "";
        while (buffer[i] != 0) {
            tmp += buffer[i];
            i++;
        }
        int serverPort = stoi(tmp);
    
        // Wait to make sure server's TCP socket is ready to accept connection.
        sleep(1);

        int sock_tcp = network->createTCPSocket();
        network->initiateTCPConnection(servaddr.sin_addr.s_addr, serverPort, sock_tcp);
        network->receiveFile(fileName, sock_tcp);

        close(sock_tcp);
    }
    
	close(sock_udp);
	return 0;
}
