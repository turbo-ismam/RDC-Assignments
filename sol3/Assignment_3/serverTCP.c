#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MAX_BUF_SIZE 33000
#define BACK_LOG 2 // Maximum queued requests
#define MIN_PROBES 20
#define MAX_BYE_PHASE 1
#define ADDR_PARAM 1
#define PORT_PARAM 2
#define HELLO_PHASE_ERROR 1
#define MEASUREMENT_PHASE_ERROR 2
#define HELLO_PHASE_OK 1
#define BYE_PHASE_OK 2
#define MIN_WORDS_MESSAGE 4
#define MAX_WORDS_MESSAGE 5

//Global variable
int n_pack = 1;
int n_probes = 0;
int server_delay = 0;

//Set the error message to send
void parsing_error(int newsfd, int errortype);
//Set the ok message to send
void parsing_ok(int newsfd, int respondtype);
//Convert char to integer
int parse_integer(char* param);
//Check the message send by client
int parse_protocol(char string[]);
//Close sockets
void close_connection(int newsfd,int sfd);
//Count how many occurences there are in a string
int count_occurences(char* string, char c);
//Print string of length numBytes
void printData(char *str, size_t numBytes);

//printData implementation
void printData(char *str, size_t numBytes){
	for(int i = 0; i < numBytes; i++){
		printf("%c", str[i]);
	}
	//printf("\n");
}

//parsing_error implementation
void parsing_error(int newsfd, int errortype){
	char response_message[MAX_BUF_SIZE];
	char *str;
	switch(errortype) { /* Based on the phase */
		case HELLO_PHASE_ERROR:
			str = "404 ERROR – Invalid Hello message\n\0";
			break;
		case MEASUREMENT_PHASE_ERROR:
			str = "404 ERROR – Invalid Measurement message\n\0";
			break;
	}
	strcpy(response_message, str);
	send(newsfd, response_message, strlen(response_message), 0);
}

//parsing_ok implementation
void parsing_ok(int newsfd, int respondtype){
	char response_message[MAX_BUF_SIZE];
	char *str;
	switch(respondtype) { /* Based on the phase */
		case HELLO_PHASE_OK:
			str = "200 OK - Ready\n\0";
			break;
		case BYE_PHASE_OK:
			str = "200 OK - Closing\n\0";
			break;
	}
	strcpy(response_message, str);
	send(newsfd, response_message, strlen(response_message), 0); 
}

//count_occurences implementation
int count_occurences(char string[MAX_BUF_SIZE], char c) 
{ 
    int res = 0; 
    int i = 0;
    for (i=0; i < strlen(string); i++) { /* Count how many occurences */
        if (string[i] == c) {
            res++; 
		}
	}
    return res; 
} 

//bye_phase implementation
void bye_phase(int newsfd){
	
	char receivedData[MAX_BYE_PHASE];
	size_t byteRecv;
	byteRecv = recv(newsfd, receivedData, sizeof(receivedData), 0);
	printf("Received data: ");
	printData(receivedData, byteRecv);
	printf("\n");
	if (byteRecv < 0){
		printf("Error recv!!");
		exit(EXIT_FAILURE);
	}
	printf("Byte received: %zd\n",byteRecv);
	if (receivedData[0] == 'b' || byteRecv <= MAX_BYE_PHASE){ /* Check the last message from Client */
		parsing_ok(newsfd, BYE_PHASE_OK);
	}
}

//parse_integer implementation
int parse_integer(char * param){
	int val = atoi(param);
	return val;
}

//parse_protocol implementation
int parse_protocol(char string[]){
	int count = count_occurences(string, ' ');
	if (count < MIN_WORDS_MESSAGE - 1 || count > MAX_WORDS_MESSAGE - 1) { /* Check if there are almost 4 words and max 5 words */
		return 0;
	}
	char *f_param = strtok(string," ");
	if (string[0] != 'h'){ /* The first parameter must be 'h' */
		return 0;
	} else {
		char *s_param = strtok(NULL," ");
		if(strcmp(s_param, "rtt") != 0 && strcmp(s_param, "thput") != 0){ /* Check if second word is rtt or thput */
			return 0;
		}
		char *t_param = strtok(NULL," ");
		int val;
		val = parse_integer(t_param);
		if (val >= MIN_PROBES){ /* The number of probes must be almost 20 */
			n_probes = val;
		} else {
			return 0;
		}
		char *f_param = strtok(NULL," ");
		val = parse_integer(f_param);
		if (val <= 0){ /* The size of the message cannot be less or equal 0 */
			return 0;
		}
		server_delay = 0; /* Default server delay */
		if (count == MAX_WORDS_MESSAGE - 1) {
			char *d_param = strtok(NULL," ");
			server_delay = parse_integer(d_param);
			if (server_delay < 0) { /* Server delay cannot be less than 0 */
				return 0;
			}
		}
		printf("\n");
	}
	return 1;
}

//measure_phase implementation
void measure_phase(int newsfd, int sfd){
	
	size_t byteRecv;
	size_t byteSent;
	char receivedData[MAX_BUF_SIZE];
	char temp_string[MAX_BUF_SIZE];
	
	while(n_pack < n_probes + 1){ /* Send messages until not reach n_probes */
		byteRecv = recv(newsfd, receivedData, sizeof(receivedData), 0);
		if (byteRecv < 0){
			printf("Error recv!!");
			exit(EXIT_FAILURE);
		}
		printf("Received data: ");
		printData(receivedData,strlen(receivedData));
		strcpy(temp_string,receivedData);
		strtok(temp_string," ");
		int n_arrived = atoi(strtok(NULL," "));
		printf("Number arrived: %d, Expected number: %d\n", n_arrived,n_pack);
		if (n_arrived != n_pack){ /* Check that the number arrived is equal to expected number */
			printf("Error sequence number!\n");
			parsing_error(newsfd, MEASUREMENT_PHASE_ERROR);
			close_connection(newsfd,sfd);
			exit(EXIT_FAILURE);
		} else {
			printf("Response to be sent back to client: ");
			printData(receivedData, strlen(receivedData));
			sleep(server_delay); /* Server delay */
			byteSent = send(newsfd, receivedData, byteRecv, 0);
			if(byteSent != byteRecv){ /* Bytes sent and bytes received must be equals */
				printf("Error send!");
				close_connection(newsfd,sfd);
				exit(EXIT_FAILURE);
			}
			if(n_pack > n_probes) { /* Check that the number of messages don't exceed the n_probes specified */
				parsing_error(newsfd, MEASUREMENT_PHASE_ERROR);
				close_connection(newsfd,sfd);
				exit(EXIT_FAILURE);
			}
			n_pack += 1;
		}
	}
}

//close_connection implementation
void close_connection(int newsfd,int sfd){
	close(sfd);
	close(newsfd);	
}

int main(int argc, char *argv[]){
	struct sockaddr_in server_addr; // struct containing server address information
	struct sockaddr_in client_addr; // struct containing client address information
	int sfd; // Server socket filed descriptor
	int newsfd; // Client communication socket - Accept result
	int br; // Bind result
	int lr; // Listen result
	int i;
	int stop = 0;
	ssize_t byteRecv; // Number of bytes received
	ssize_t byteSent; // Number of bytes to be sent
	socklen_t cli_size;
	char receivedData [MAX_BUF_SIZE]; // Data to be received
	char sendData [MAX_BUF_SIZE]; // Data to be sent
	char port[MAX_BUF_SIZE]; //server port
	char addr[MAX_BUF_SIZE]; //server addr
	char client_name[MAX_BUF_SIZE]; //Name of the client connected
	printf("Server addr: "); /* Request ip address of server */
	scanf("%s",addr);
	printf("Server port: "); /* Request port of server */
	scanf("%s",port);
	sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sfd < 0){
		printf("socket error"); // Print error message
		exit(EXIT_FAILURE);
	}
	// Initialize server address information
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(port));
	server_addr.sin_addr.s_addr = inet_addr(addr);
	br = bind(sfd, (struct sockaddr *) &server_addr, sizeof(server_addr));
	if (br < 0){
		printf("bind error"); //Print error message
		exit(EXIT_FAILURE);
	}
	cli_size = sizeof(client_addr);
	// Listen for incoming requests
	printf("In ascolto\n");
	lr = listen(sfd, BACK_LOG);
	if (lr < 0){
		printf("listen error"); // Print error message
		exit(EXIT_FAILURE);
	}
	for(;;){
	// Wait for incoming requests
		newsfd = accept(sfd, (struct sockaddr *) &client_addr, &cli_size);
		if (newsfd < 0){
			printf("accept error"); // Print error message
			exit(EXIT_FAILURE);
		}
		inet_ntop(AF_INET, &client_addr.sin_addr, client_name, cli_size); /* Get the ip address of the connected client */
		printf("Client accepted, IP address: %s, port: %d\n",  client_name, ntohs(client_addr.sin_port)); /* Print connected client information */
		while(1){
			byteRecv = recv(newsfd, receivedData, sizeof(receivedData), 0);
			if (byteRecv < 0){
				printf("recv error");
				exit(EXIT_FAILURE);
			}
			printf("Received data: ");
			printData(receivedData, byteRecv);
			if(strncmp(receivedData, "exit", byteRecv) == 0){
				printf("Command to stop server received\n");
				close_connection(newsfd,sfd);
				exit(EXIT_SUCCESS);
			}
			if(parse_protocol(receivedData)){ /* Check if Hello message is ok, if it's ok continue with others phases */
				parsing_ok(newsfd, HELLO_PHASE_OK);
				measure_phase(newsfd,sfd);
				bye_phase(newsfd);
				close_connection(newsfd,sfd);
				exit(EXIT_SUCCESS);
			} else { /* If hello message is wrong send error message */
				parsing_error(newsfd, HELLO_PHASE_ERROR);
				close_connection(newsfd,sfd);
				exit(EXIT_FAILURE);
			}	
		}
	} // End of for(;;)
	close(sfd);
	return 0;
}
