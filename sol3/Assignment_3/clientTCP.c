#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/time.h>
#define MAX_BUF_SIZE 33000
#define OK 200
#define MAX_BYE_PHASE 10
#define MAX_NUMBER 9
#define DECIMAL_BASE 10
#define ADDR_PARAM 1
#define PORT_PARAM 2
#define SERVER_ADDRESS 20
#define MAX_MEASURE_CHARS 5
#define SECOND_TO_MICROSECOND 1000000
#define SECOND_TO_MILLISECOND 1000
#define BYTE_TO_KILOBYTE 1024
#define CONVERSION_KB 1024
#define LAST_CHARS 2

//Global var
int n_pack = 1;

//Struct to save information about message send to the server
typedef struct {
	char measure_type[MAX_MEASURE_CHARS];
	int n_probes;
	int msg_size;
	int server_delay; 
}protocol_msg;

protocol_msg msg;

struct sockaddr_in server_addr; // struct containing server address information

//Clean stdin
void clean_stdin(void);
//Manage bye phase
void bye_phase(int sfd);
//Save result of measurement phase on file
void saving_result(double value);
//Measure RTT or Throughput
void measure_phase(int sfd);
//print exchanged data 
void printData(char *str, size_t numBytes);

//clean_stdin implementation
void clean_stdin(void)
{
    int c;
    do {
        c = getchar();
    } while (c != '\n' && c != EOF);
}

//printData implementation
void printData(char *str, size_t numBytes){
	for(int i = 0; i < numBytes; i++){
		printf("%c", str[i]);
	}
	//printf("\n");
}

//bye_phase implementation
void bye_phase(int sfd){
	char bye_phase[MAX_BYE_PHASE];
	char msg[MAX_BUF_SIZE];
	size_t byteSent;
	size_t byteRecv;
	strcpy(bye_phase,"b\n\0"); //bye message
	printf("String going to be sent to server: %s\n", bye_phase);
	byteSent = sendto(sfd, bye_phase, strlen(bye_phase), 0, (struct sockaddr *)
	&server_addr, sizeof(server_addr));
	byteRecv = recv(sfd, msg, MAX_BUF_SIZE, 0);
	printf("Received from server: ");
	printData(msg, byteRecv);
	int code = atoi(strtok(bye_phase, " "));
	close(sfd); //Close connection
	if (code == OK){
		exit(EXIT_SUCCESS);
	} else {
		exit(EXIT_FAILURE);
	}
}

//saving_result implementation
//Rtt and throughput results are written on a .txt file 
//in order to make graphs
void saving_result(double value){
	
	FILE* fp;
	char name_file[MAX_BUF_SIZE];
	strcpy(name_file,msg.measure_type);
	strcat(name_file,".txt");
	fp = fopen(name_file,"a");
	if (fp == NULL){
		printf("Fopen error!");
		exit(EXIT_FAILURE);
	}
	fprintf(fp,"%f\n",value); 
	fclose(fp);
}

//measure_phase implementation
void measure_phase(int sfd){
	char receivedData[MAX_BUF_SIZE];
	size_t byteSent;
	size_t byteRecv;
	struct timeval start;
	struct timeval stop;
	double rtt;
	double thput;
	double avg_rtt;
	while(n_pack < msg.n_probes + 1){
		char vect[MAX_BUF_SIZE] = {0};
		char number_buffer[MAX_NUMBER];
		int k = 0;
		strcpy(vect,"m "); //write <protocol phase> section 
		sprintf(number_buffer,"%d ",n_pack); //then <sequence number section>
		strcat(vect, number_buffer); 
		int j = 0;
		char payload[msg.msg_size + LAST_CHARS];
		while(j < msg.msg_size){ //and at last, write <payload> section
			payload[j] = '*';
			j += 1;
		}
		payload[j] = '\n';
		strcat(vect, payload);
		strcat(vect,"\0");
		gettimeofday(&start,NULL); //get start time
		byteSent = sendto(sfd, vect, strlen(vect), 0, (struct sockaddr *) &server_addr, sizeof(server_addr)); //then send file
		byteRecv = recv(sfd, receivedData, sizeof(receivedData), 0); //receive bytes
		gettimeofday(&stop,NULL); //get last time
		rtt = ((stop.tv_sec) * SECOND_TO_MICROSECOND + (stop.tv_usec)) - 
			  ((start.tv_sec) * SECOND_TO_MICROSECOND + (start.tv_usec)); //time measured in microseconds
		rtt /= SECOND_TO_MILLISECOND; //then converted to milliseconds
		avg_rtt += rtt;
		printf("Sequence number: %s\n",number_buffer); //print sequence number
		printf("Rtt: %f\n",rtt);
		printf("Byte recv: %zd, byte sent: %zd\n",byteRecv, byteSent);
		if (byteRecv < 0 || byteSent != byteRecv){
			printf("Error in data transfer!!");
			exit(EXIT_FAILURE);
		}
		n_pack += 1;
	}
	avg_rtt /= msg.n_probes; //compute average Rtt
	if(strcmp(msg.measure_type,"rtt") == 0){
		printf("L'rtt medio e\': %f ms\n",avg_rtt);
		saving_result(avg_rtt);
	} else {
		thput = (byteRecv/BYTE_TO_KILOBYTE) / (avg_rtt/SECOND_TO_MILLISECOND);//compute thput 
		printf("Il throwput e\': %f KB/S\n",thput);
		saving_result(thput);
	}
}

int main(int argc, char *argv[]){
	int sfd; // Server socket filed descriptor
	int cr; // Connect result
	int stop = 0;
	ssize_t byteRecv; // Number of bytes received
	ssize_t byteSent; // Number of bytes to be sent
	char receivedData [MAX_BUF_SIZE] = {0}; // Data to be received
	char sendData [MAX_BUF_SIZE]; // Data to be sent
	char port [MAX_BUF_SIZE];// server port
	char addr [MAX_BUF_SIZE];// addr port
	printf("Server addr: ");
	scanf("%s",addr); //address and port for tcp handshake are taken by stdin
	printf("Server port: ");
	scanf("%s", port);
	clean_stdin();
	sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
	if (sfd < 0){
		printf("Scoket error"); //Print error message
		exit(EXIT_FAILURE);
	}
	server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(port));
    server_addr.sin_addr.s_addr = inet_addr(addr);
	cr = connect(sfd, (struct sockaddr *) &server_addr, sizeof(server_addr));
	if (cr < 0){
		printf("Connect error"); //Print error message
		exit(EXIT_FAILURE);
	}
	while(!stop){
		printf("Insert message: ");
		scanf("%[^\n]s",sendData);// hello message is inserted by stdin.
		clean_stdin();
		printf("String going to be sent to server: %s\n", sendData);
		if(strcmp(sendData, "exit") == 0){
			stop = 1;
		}
		byteSent = sendto(sfd, sendData, strlen(sendData), 0, (struct sockaddr *) &server_addr, sizeof(server_addr));
		printf("Bytes sent to server: %zd\n", byteSent);
		if(!stop){
			byteRecv = recv(sfd, receivedData, MAX_BUF_SIZE, 0);
			printf("Received from server: ");
			printData(receivedData, byteRecv);
			int code = atoi(strtok(receivedData, " ")); //extract code from server's response message
			if (code == OK){ //Parameters are written in data structure
				strtok(sendData," ");
				strcpy(msg.measure_type,strtok(NULL," "));
				msg.n_probes = atoi(strtok(NULL," "));
				msg.msg_size = atoi(strtok(NULL," "));
				measure_phase(sfd);
				bye_phase(sfd);
			} else {
				exit(EXIT_FAILURE);
			}
		}
	} // End of while
	close(sfd);
	return 0;
}
