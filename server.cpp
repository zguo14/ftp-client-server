#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include "network.h"

#define MAXLEN 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cout<<argc<<std::endl;
        std::cout<<"Invalid params, usage: ./server.sh <req_code> <file_to_send>"<<std::endl;
        exit(EXIT_FAILURE);
    }
    
    Network *network = new Network();

    std::string reqCode = argv[1];
    char * fileToSend = argv[2];

    if (!network->checkFileExist(fileToSend)) {
        std::cout<<"File does not exist."<<std::endl;
        exit(EXIT_FAILURE);
    }
   
    int sock_udp;
	char buffer[MAXLEN];
    int serverUDPPort = network->getFreePort(SOCK_DGRAM);
    std::cout<<"SERVER_PORT="<<serverUDPPort<<std::endl;

	struct sockaddr_in servaddr, cliaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	memset(&cliaddr, 0, sizeof(cliaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(serverUDPPort);

    sock_udp =  network->createUDPSocket();
    network->bindUDPSocket(sock_udp, servaddr);

    // Server continuously accept UDP connections.
    while (1) {
        std::cout<<"Server listening...."<<std::endl;

        network->recvFromUDPSocket(sock_udp, cliaddr, buffer);
        
        std::string mode;
        int clientPort = 0;
        std::string clientReqCode;
        int i;

        // Extract mode(PORT/PASV) from buffer.
        for (i = 0; i < 4; i++) {
            mode += buffer[i];
        }
        i++;
        // Extract <r_port> and <req_code> from buffer.
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
        // Extract <req_code> from buffer.
        if (mode == "PASV") {
            while (i < sizeof(buffer) && buffer[i] != ' ') {
                clientReqCode += buffer[i];
                i++;
            }
        }

        // Client sends correct req_code.
        if (clientReqCode == reqCode) {
            // Active mode.
            if (mode == "PORT") {
                const char *resp = "1";
                network->sendToUDPSocket(sock_udp, resp, cliaddr);

                // Wait to make sure client's TCP socket is ready to accept connection.
                sleep(1);

                int sock_tcp = network->createTCPSocket();
                network->initiateTCPConnection(cliaddr.sin_addr.s_addr, clientPort, sock_tcp);
                network->sendFile(fileToSend, sock_tcp);

                close(sock_tcp);
            }
            
            // Passive mode.
            if (mode == "PASV") {
                int port = network->getFreePort(SOCK_STREAM);
                char *resp = strdup(std::to_string(port).c_str());
                network->sendToUDPSocket(sock_udp, resp, cliaddr);

                int sock_tcp = network->createTCPSocket();
                int sock_new = network->acceptTCPConnection(port, sock_tcp);
                network->sendFile(fileToSend, sock_new);

                close(sock_new);
                close(sock_tcp);
            }
        } else {
            // Client sends incorrect req_code, respond with 0.
            const char *resp = "0";
            network->sendToUDPSocket(sock_udp, resp, cliaddr);
        }
    }

	close(sock_udp);
	return 0;
}
