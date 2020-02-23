#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>

#define MAX_BUF_SIZE 50000 // Maximum size of UDP messages
#define SERVER_PORT 7777
#define OK 200
#define ERROR 404

void printData(char *str, size_t numBytes){
	for(int i = 0; i < numBytes; i++){
		printf("%c", str[i]);
	}
	printf("\n");
}

int main(int argc, char *argv[]){
  struct sockaddr_in server_addr; // struct containing server address information
  struct sockaddr_in client_addr; // struct containing client address information
  int sfd; // Server socket filed descriptor
  int cr; // Connect result
  int stop = 0;
  int n;
  float rtt, total_rtt;
  ssize_t byteRecv; // Number of bytes received
  ssize_t byteSent; // Number of bytes sent
  size_t msgLen; 
  socklen_t serv_size;
  int msg_size, n_probes;
  char measureType[MAX_BUF_SIZE];
  char receivedData [MAX_BUF_SIZE]; // Data to be received
  char sendData [MAX_BUF_SIZE]; // Data to be sent
  char message [MAX_BUF_SIZE];
  char payload [MAX_BUF_SIZE];
  
  sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  
  if (sfd < 0){
  	perror("socket"); // Print error message
  	exit(EXIT_FAILURE);
  }
  
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(SERVER_PORT);
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  
  serv_size = sizeof(server_addr);

  cr = connect(sfd, (struct sockaddr *) &server_addr, sizeof(server_addr));

  if (cr < 0){
    perror("connect"); // Print error message
    exit(EXIT_FAILURE);
  }

  
  while(!stop){
    strcpy(sendData, "h rtt 20 1000 0\n\0"); //<protocol_phase> <sp> <measure_type> rtt/thput <sp> <n_probes> <sp> <msg_size> <sp> <server_delay>
  	printf("String sent to server: %s\n", sendData);
  	
  	if(strcmp(sendData, "exit") == 0){
  		stop = 1;
  	}
  	strcat(sendData, "\n");
  	msgLen = strlen(sendData);
  
  	byteSent = send(sfd, sendData, msgLen, 0);
  	printData(sendData, byteSent);
  	
  	if(!stop)
  	{
  		byteRecv = recv(sfd, receivedData, MAX_BUF_SIZE, 0);
  		printf("Received from server:");
  		printData(receivedData, byteRecv);

		char temp[MAX_BUF_SIZE];
		strcpy(temp, receivedData);
		int code = atoi(strtok(temp, " "));
		printf("code : %d\n", code);
		fflush(stdout);
		if(code == OK) //measure phase
		 {
			strtok(sendData," ");
			strcpy(measureType,strtok(NULL," "));
			n_probes = atoi(strtok(NULL," "));
			msg_size = atoi(strtok(NULL," "));
			n = 0;
			while(n < msg_size)
			{
					payload[n] = 'F';
			  n++;
				}
			payload[n]= '\0';
			n = 1;
			char buffer [MAX_BUF_SIZE];
			while (n <= n_probes)
			{ 
			  sprintf(buffer, "%d", n);
			  strcpy(message, "m ");
			  strcat(message, buffer);
			  strcat(message, " ");
			  strcat(message, payload);
			  strcat(message, "\n\0");
			  msgLen= strlen(message);
			  clock_t start = clock();
			  byteSent = send(sfd, message, msgLen, 0);
			  printf("packet %d sent\n", n);
			  byteRecv = recv(sfd, receivedData, sizeof(receivedData), 0);
			  printf("packet %d received\n", n);
			  clock_t end = clock();
			  rtt = (double)(end - start) / CLOCKS_PER_SEC;
			  total_rtt += rtt;
			  printf("RTT: %f microseconds\n", rtt*1000);
			  n++;
			}
			if(strcmp(measureType,"rtt") == 0)
			{
				printf("Avg rtt: %f microseconds\n", total_rtt/n_probes*1000);
			}
			else
			{
			  printf("Avg thput: %f Kbyte/s\n", (strlen(message)/1024)/(total_rtt/n_probes));
			}
			//bye phase
			strcpy(sendData, "b\n\0");
			printf("Message sent to server: %s", sendData);
			byteSent = send(sfd, sendData, strlen(sendData), 0);
			byteRecv = recv(sfd, receivedData, sizeof(receivedData), 0);
			printf("Server response : ");
			printData(receivedData, byteRecv);
			close(sfd);
			exit(EXIT_SUCCESS);
		}	
		else 
		{
		   exit(EXIT_FAILURE);
		}
  	}
  }
  close(sfd);
  return 0;
}
