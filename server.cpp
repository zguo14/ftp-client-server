#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include <vector>
#include "network.h"

#define MAXLEN 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cout<<argc<<std::endl;
        std::cout<<"Invalid params, usage: ./server.sh <req_code> <file_to_send>"<<std::endl;
        exit(EXIT_FAILURE);
    }
    
    std::string reqCode = argv[1];
    char * fileToSend = argv[2];

    Network *network = new Network();

    int serverUDPPort = network->getFreePort(SOCK_DGRAM);
    std::cout<<"SERVER_PORT="<<serverUDPPort<<std::endl;
    
	int sockfd;
	char buffer[MAXLEN];
	struct sockaddr_in servaddr, cliaddr;

	memset(&servaddr, 0, sizeof(servaddr));
	memset(&cliaddr, 0, sizeof(cliaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
    // servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = htons(serverUDPPort);

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        std::cout<<"UDP socket creation failed"<<std::endl;
		exit(EXIT_FAILURE);
	}

	if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ) {
        std::cout<<"UDP socket bind failed"<<std::endl;
		exit(EXIT_FAILURE);
	}

    while (1) {
        std::cout<<"Server listening...."<<std::endl;
        socklen_t len = sizeof(cliaddr);
        int n = recvfrom(sockfd, (char *)buffer, MAXLEN,
				MSG_WAITALL, ( struct sockaddr *) &cliaddr, &len);
        
        std::string mode;
        int clientPort = 0;
        std::string clientReqCode;
        int i;

        for (i = 0; i < 4; i++) {
            mode += buffer[i];
        }

        i++;

        if (mode == "PORT") {
            while (i < sizeof(buffer) && buffer[i] != ' ') {
                clientPort = (buffer[i] - '0') + clientPort * 10;
                i++;
            }

            i++;

            while (i < sizeof(buffer) && buffer[i] != ' ') {
                clientReqCode += buffer[i];
                i++;
            }
        }
        
        if (mode == "PASV") {
            while (i < sizeof(buffer) && buffer[i] != ' ') {
                clientReqCode += buffer[i];
                i++;
            }
        }

        // std::cout<<"mode is: "<<mode<<std::endl;
        // std::cout<<"reqcode is:"<<clientReqCode.c_str()<<std::endl;
        // std::cout<<"clieint port is:"<<clientPort<<std::endl;
        // Send file to client
        if (clientReqCode == reqCode) {
            if (mode == "PORT") {
                const char *resp = "1";
                // std::cout<<"into port"<<std::endl;
                sendto(sockfd, (const char *)resp, strlen(resp), 0, 
                (const struct sockaddr *) &cliaddr,len);

                // std::cout<<"sent confirm to client"<<std::endl;
                sleep(1);
                // std::cout<<"wake up"<<std::endl;

                int sock_tcp = network->createTCPSocket();
                network->initiateTCPConnection(cliaddr.sin_addr.s_addr, clientPort, sock_tcp);

                network->sendFile(fileToSend, sock_tcp);

                // std::cout<<"file sent!"<<std::endl;

                close(sock_tcp);
            }

            if (mode == "PASV") {
                int port = network->getFreePort(SOCK_STREAM);
                const char *resp = strdup(std::to_string(port).c_str());
                // std::cout<<"into pasv"<<std::endl;
                sendto(sockfd, (const char *)resp, strlen(resp), 0, 
                (const struct sockaddr *) &cliaddr,len);
                // std::cout<<"sent port num to client"<<std::endl;
                int sock_tcp = network->createTCPSocket();
                // std::cout<<"tcp socket created"<<std::endl;
                int sock_new = network->acceptTCPConnection(port, sock_tcp);
                // std::cout<<"tcp socket accepted"<<std::endl;
                network->sendFile(fileToSend, sock_new);

                close(sock_new);
                close(sock_tcp);
            }
        } else {
            const char *resp = "0";
            sendto(sockfd, (const char *)resp, strlen(resp), 0, 
            (const struct sockaddr *) &cliaddr,len);
        }
    }

	close(sockfd);

	return 0;
}
