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

    Network *network = new Network();

	char buffer[MAXLINE];
	struct sockaddr_in	 servaddr;
    char* serverAddr = argv[1];
    char* serverPort = argv[2];
    char* mode = argv[3];
    char* reqCode = argv[4];
    char* fileName = argv[5];

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

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
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

    // std::cout<<mode<<std::endl; 
    if (strcmp(mode, "PORT") == 0) {
        // std::cout<<"into port branch"<<std::endl;
        int port = network->getFreePort(SOCK_STREAM);
        std::string cmd = "PORT " + std::to_string(port) + " " + (std::string)reqCode + " ";
        request = cmd.c_str();

        // std::cout<<"got free port, cmd is: "<<cmd<<std::endl;

        sendto(sockfd, (const char *)request, strlen(request), 0, 
            (const struct sockaddr *) &servaddr,sizeof(servaddr));

        // std::cout<<"udp datagram sent "<<std::endl;

        n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, 
            (struct sockaddr *) &servaddr, &len);

        // std::cout<<"udp datagram received, buffer is: "<<buffer<<std::endl;

        if (buffer[0] == '0') {
            std::cout<<"Wrong req_code."<<std::endl;
            exit(EXIT_FAILURE);
        }

        // std::cout<<"Correct code, returned 1"<<std::endl;
    
        int sock_tcp = network->createTCPSocket();
        int sock_new = network->acceptTCPConnection(port, sock_tcp);

        network->receiveFile(fileName, sock_new);

        close(sock_new);
        close(sock_tcp);
    }

    if (strcmp(mode, "PASV") == 0) {
        // std::cout<<"into pasv branch"<<std::endl;
        std::string cmd = "PASV " + (std::string)reqCode + " ";
        request = cmd.c_str();

        // std::cout<<"pasv request is: "<<request<<std::endl;

        sendto(sockfd, (const char *)request, strlen(request), 0, 
            (const struct sockaddr *) &servaddr,sizeof(servaddr));

            // std::cout<<"udp datagram sent "<<std::endl;

        n = recvfrom(sockfd, (char *)buffer, MAXLINE, 0, 
            (struct sockaddr *) &servaddr, &len);

        //  std::cout<<"udp datagram received, buffer is: "<<buffer<<std::endl;
        // std::cout<<"buffer "<<buffer<<std::endl;

        if (buffer[0] == '0') {
            std::cout<<"Wrong req_code."<<std::endl;
            exit(EXIT_FAILURE);
        }
        
        int i = 0;
        std::string tmp = "";
        while (buffer[i] != 0) {
            tmp += buffer[i];
            i++;
        }

        int serverPort = stoi(tmp);

        sleep(1);
        // std::cout<<"awake."<<std::endl;
        // std::cout<<"tcp to server addr is: "<<servaddr.sin_addr.s_addr<<std::endl;
        // std::cout<<"tcp to server port is: "<<serverPort<<std::endl;
        int sock_tcp = network->createTCPSocket();
        network->initiateTCPConnection(servaddr.sin_addr.s_addr, serverPort, sock_tcp);
        // std::cout<<"tcp initiated."<<std::endl;
        int size = network->receiveFile(fileName, sock_tcp);
        // std::cout<<"received, size is: "<<size<<std::endl;

        close(sock_tcp);
    }
    
	close(sockfd);
	return 0;
}
