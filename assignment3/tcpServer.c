#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MAX_BUF_SIZE 1024 
#define SERVER_PORT 7777

#define BACK_LOG 2

int main(int argc, char *argv[])
{
	struct sockaddr_in client_addr; // struct containing client address information
	struct sockaddr_in server_addr; // struct containing server address information
	int sfd; // Socket file descriptor
	int newsfd; //client communication socket - accept result
	int errorFlag=0;
	int words;
	int i;
	int br, lr; //error control params
	ssize_t byteRecv; // Number of bytes received
	ssize_t byteSent; // Number of bytes to be sent
	socklen_t cli_size;
	char receivedData [MAX_BUF_SIZE]; // Data to be received
	char sendData [MAX_BUF_SIZE]; // Data to be sent
	char extractData [MAX_BUF_SIZE];
	char message[MAX_BUF_SIZE];
	char clientName[50];

	// Initialize server address information 
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT); // Convert to network byte order
	server_addr.sin_addr.s_addr = INADDR_ANY; // Bind to any address
	
	sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sfd < 0)
	{
		perror("socket"); // Print error message
		exit(EXIT_FAILURE);
	}

	br = bind(sfd, (struct sockaddr *) &server_addr, sizeof(server_addr));
	if (br < 0)
	{
		perror("bind"); // Print error message
		exit(EXIT_FAILURE);
	}
	cli_size = sizeof(client_addr);
	// Listen for incoming requests
	lr = listen(sfd, BACK_LOG);
	if (lr < 0)
	{
		printf("listen error"); // Print error message
		exit(EXIT_FAILURE);
	}
	printf("listening...");
	fflush(stdout);
	
	for(;;) // Wait for incoming requests
	{ 
		newsfd = accept(sfd, (struct sockaddr *) &client_addr, &cli_size);
		if (newsfd < 0)
		{
			perror("accept"); // Print error message
			exit(EXIT_FAILURE);
		}
		inet_ntop(AF_INET, &client_addr.sin_addr, clientName, cli_size); /* Gets ip of client */
		printf("Client IP    mn address: %s, port: %d\n",  clientName, ntohs(client_addr.sin_port));
		while(1)
		{
			byteRecv = recv(newsfd, receivedData, sizeof(receivedData), 0);
			if (byteRecv < 0)
			{
				perror("recv");
				exit(EXIT_FAILURE);
			}
			printf("Received data: ");
			printf("%s %lu\n",receivedData, byteRecv);
			words=0;
			for (i=0; i < strlen(receivedData); i++)  //Count words
			{
				if (receivedData[i] == ' ') 
				{
					words++;
				}
			}
			if(strncmp(receivedData, "exit", byteRecv) == 0)
			{
				printf("Command to stop server received\n");
				close(newsfd);
				break;
			}
			if(words==4)
			{
				char * phase = strtok(receivedData," ");
				if(strcmp(phase, "h")!=0)
				{
					errorFlag=1;
				}
				char * type = strtok(NULL, " ");
				if(strcmp(type, "rtt")!=0 && strcmp(type, "thput")!=0)
				{
					errorFlag=1;
				}
				char * n_probes = strtok(NULL, " ");
				if(atoi(n_probes)<0)
				{
					errorFlag=1;
				}
				char * msg_size = strtok(NULL, " ");
				if(atoi(msg_size)<0)
				{
					errorFlag=1;
				}
				char * server_delay = strtok(NULL, " ");
				if(atoi(server_delay)<0)
				{
					errorFlag=1;
				}
				if(errorFlag==0)
				{
					message = "200 OK - Ready";
					send(newsfd, message, strlen(message), 0);
					int n = 1;
					size_t MeasureByteRecv;
					size_t MeasureByteSent;
					char receivedData[MAX_BUF_SIZE];
					char temp_string[MAX_BUF_SIZE];
					while(n <= n_probes)
					{
						byteRecv = recv(newsfd, receivedData, sizeof(receivedData), 0);
						strcpy(extractData, receivedData)
						strtok(extractData, " ");
						receivedNumber = atoi(strtok(NULL, " "));
						printf("Received number: %d, Expected number: %d\n", receivedNumber,n);
						if (n_arrived != n_pack)
						{
							printf("Error sequence number!\n");
							close(newsfd);
							close(sfd);
							exit(EXIT_FAILURE);
						} 
						else 
						{
							sleep(server_delay);
							printf("Measure response: %s", receivedData);
							byteSent = send(newsfd, receivedData, byteRecv, 0);
						}
						n++;
					}
					//bye phase
					byteRecv = recv(newsfd, receivedData, sizeof(receivedData), 0);
					printf("Received data: %s", receivedData);
					close(sfd);
					close(newsfd);
					exit(EXIT_SUCCESS);

				}
				else 
				{
					message = "404 ERROR - Invalid hello message";
					send(newsfd, message, strlen(message), 0);
					close(sfd);
					close(newsfd);
					exit(EXIT_FAILURE);
				}
			}
			else
			{
				exit(EXIT_FAILURE);
			}
			printf("Response to be sent back to client: ");
			//printData(receivedData, byteRecv);
			byteSent = send(newsfd, receivedData, byteRecv, 0);
		}
	} // End of for(;;)
		close(sfd);
		return 0;
}


