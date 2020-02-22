#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MAX_BUF_SIZE 1024 // Maximum size of UDP messages
#define SERVER_PORT 7777
#define OK 200
#define ERROR 404


int main(int argc, char *argv[]){
  struct sockaddr_in server_addr; // struct containing server address information
  struct sockaddr_in client_addr; // struct containing client address information
  int sfd; // Server socket filed descriptor
  int cr; // Connect result
  int stop = 0;
  float rtt, total_rtt;
  ssize_t byteRecv; // Number of bytes received
  ssize_t byteSent; // Number of bytes sent
  size_t msgLen; 
  socklen_t serv_size;
  int msg_size, n_probes;
  char measureType[10];
  char receivedData [MAX_BUF_SIZE]; // Data to be received
  char sendData [MAX_BUF_SIZE]; // Data to be sent
  char message [MAX_BUF_SIZE];
  
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
    strcpy(sendData, "h rtt 5 10 0\n\0"); //<protocol_phase> <sp> <measure_type> rtt/thput <sp> <n_probes> <sp> <msg_size> <sp> <server_delay>
  	printf("String going to be sent to server: %s\n", sendData);
  	strtok(sendData," ");
        strcpy(measureType,strtok(NULL," "));
			  n_probes = atoi(strtok(NULL," "));
				msg_size = atoi(strtok(NULL," "));
  	if(strcmp(sendData, "exit") == 0){
  		stop = 1;
  	}
  	strcat(sendData, "\n");
  	msgLen = strlen(sendData);
  
  	byteSent = send(sfd, sendData, msgLen, 0);
  	printf("Bytes sent to server: %zd\n", byteSent);
  	
  	if(!stop){
  		byteRecv = recv(sfd, receivedData, MAX_BUF_SIZE, 0);
  		perror("Received from server: ");
  		printf("%s, %lu", receivedData, byteRecv);
     int code = strtok(receivedData, " ");
     if(code == OK)
     {
        n = 0;
        while(n < msg_size)
        {
			    payload[n] = 'F';
          n++;
		    }
        payload[n]= "\0";
        int n = 1;
        while (n <= n_probes)
        { 
          message= "m ";
          strcat(message, n);
          strcat(message, " ");
          strcat(message, payload);
          strcat(message, "\n\0");
          msgLen= strlen(message);
          clock_t start = clock();
          byteSent = send(sfd, sendData, msgLen, 0);
          byteRecv = recv(sfd, receivedData, sizeOf(receivedData), 0);
          clock_t end = clock();
          rtt = (double)(end - start) / CLOCKS_PER_SEC;
          total_rtt += rtt;
          printf("Elapsed: %f seconds\n", rtt);
          n++;
        }
        if(strcmp(msg.measure_type,"rtt") == 0)
        {
            printf("Avg rtt: %f seconds\n", total_rtt/n_probes);
        }
        else
        {
          printf("Avg thput: %f seconds\n", total_rtt/n_probes);
        }
        //bye phase
        sendData= "b\n\0";
        byteSent = send(sfd, sendData, msgLen, 0);
        byteRecv = recv(sfd, receivedData, sizeOf(receivedData), 0);
        printf("received from server : %s", receivedData );
        close(sfd);
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
