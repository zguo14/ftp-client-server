// Client side implementation of UDP client-server model
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include "network.h"

// #define PORT	 1883
#define MAXLINE 1024
#define MSG_CONFIRM 0

int main(int argc, char *argv[]) {
    if (argc != 6) {
        std::cout<<"Invalid params, usage: ./client.sh <server address> <n_port>" 
        "<mode> <req_code> \'received.txt\'"<<std::endl;
        exit(EXIT_FAILURE);
    }

    Network *network = new Network();

	int sockfd;
	char buffer[MAXLINE];
	struct sockaddr_in	 servaddr;
    char* serverAddr = argv[1];
    char* serverPort = argv[2];
    char* mode = argv[3];
    char* reqCode = argv[4];
    char* fileName = argv[5];

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		std::cout<<"UDP socket creation failed."<<std::endl;
		exit(EXIT_FAILURE);
	}

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(serverPort));
	// servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_addr.s_addr = inet_addr(serverAddr);
	
	int n;
	socklen_t len;
    const char *request;

    std::cout<<mode<<std::endl;
    if (strcmp(mode, "PORT") == 0) {
        int port = network->getFreePortInRange(1024, 65535, SOCK_STREAM);
        std::string cmd = "PORT " + std::to_string(port) + " " + (std::string)reqCode + " ";
        request = cmd.c_str();

        sendto(sockfd, (const char *)request, strlen(request), 0, 
            (const struct sockaddr *) &servaddr,sizeof(servaddr));

        n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, 
            (struct sockaddr *) &servaddr, &len);

        if (buffer[0] == '0') {
            std::cout<<"Wrong req_code."<<std::endl;
            exit(EXIT_FAILURE);
        }

        std::cout<<"Correct code, returned 1"<<std::endl;
    
        int sock_tcp = network->createTCPSocket();
        int sock_new = network->acceptTCPConnection(port, sock_tcp);

        network->receiveFile(fileName, sock_new);

        close(sock_new);
        close(sock_tcp);
    }

    if (strcmp(mode, "PASV") == 0) {
        std::string cmd = "PASV " + (std::string)reqCode + " ";
        request = cmd.c_str();

        std::cout<<request<<std::endl;
        sendto(sockfd, (const char *)request, strlen(request), 0, 
            (const struct sockaddr *) &servaddr,sizeof(servaddr));

        n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, 
            (struct sockaddr *) &servaddr, &len);

        // std::cout<<"buffer "<<buffer<<std::endl;

        if (buffer[0] == '0') {
            std::cout<<"Wrong req_code."<<std::endl;
            exit(EXIT_FAILURE);
        }

        int serverPort = 0;
        int i = 0;
        while (buffer[i] != 0) {
            serverPort = serverPort * 10 + (buffer[i] - '0');
            i++;
        }
        
        // std::cout<<"serverPort "<<serverPort<<std::endl;

        sleep(1);

        int sock_tcp = network->createTCPSocket();
        network->initiateTCPConnection(inet_addr(serverAddr), serverPort, sock_tcp);

        int size = network->receiveFile(fileName, sock_tcp);
        std::cout<<"received, size is: "<<size<<std::endl;
    }
    
	close(sockfd);
	return 0;
}
