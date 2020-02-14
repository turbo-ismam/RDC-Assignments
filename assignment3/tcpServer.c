#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "myfunction.h"

#define MAX_BUF_SIZE 1024 

#define SERVER_PORT 7777

int main(int argc, char *argv[])
{
	struct sockaddr_in client_addr; // struct containing client address information
	int sfd; // Socket file descriptor
	int newsfd; //client communication socket - accept result
	int i;
	int br, lr; //error control params
	ssize_t byteRecv; // Number of bytes received
	ssize_t byteSent; // Number of bytes to be sent
	socklen_t cli_size;
	char receivedData [MAX_BUF_SIZE]; // Data to be received
	char sendData [MAX_BUF_SIZE]; // Data to be sent
	char clientName[50];

	// Initialize server address information 
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT); // Convert to network byte order
	server_addr.sin_addr.s_addr = INADDR_ANY; // Bind to any address

	br = bind(sfd, (struct sockaddr *) &server_addr, sizeof(server_addr));
	if (br < 0)
	{
		perror("bind"); // Print error message
		exit(EXIT_FAILURE);
	}
	cli_size = sizeof(client_addr);
	// Listen for incoming requests
	lr = listen(sfd, BACK_LOG);
	if (lr < 0){
		printf("listen error"); // Print error message
		exit(EXIT_FAILURE);
	}

	for(;;) // Wait for incoming requests
	{ 
		newsfd = accept(sfd, (struct sockaddr *) &client_addr, &cli_size);
		if (newsfd < 0)
		{
			perror("accept"); // Print error message
			exit(EXIT_FAILURE);
		}
		inet_ntop(AF_INET, &client_addr.sin_addr, clientName, cli_size); /* Gets ip of client */
		printf("Client IP address: %s, port: %d\n",  clientName, ntohs(client_addr.sin_port));
		while(1)
		{
			byteRecv = recv(newsfd, receivedData, sizeof(receivedData), 0);
			if (byteRecv < 0)
			{
				perror("recv");
				exit(EXIT_FAILURE);
			}
			printf("Received data: ");
			printData(receivedData, byteRecv);
			if(strncmp(receivedData, "exit", byteRecv) == 0
			{
				printf("Command to stop server received\n");
				close(newsfd);
				break;
			}
			convertToUpperCase(receivedData, byteRecv);
			printf("Response to be sent back to client: ");
			printData(receivedData, byteRecv);
			byteSent = send(newsfd, receivedData, byteRecv, 0);
			if(byteSent != byteRecv
			{
				perror("send");
				exit(EXIT_FAILURE);
			}
		}
	} // End of for(;;)
		close(sfd);
		return 0;
}


