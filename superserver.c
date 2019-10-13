#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/time.h>
#include<netinet/in.h>
#include<signal.h>
#include<errno.h>
#define FILENAME "serverlist.txt"
//Constants and global variable declaration goes here


//Service structure definition goes here
typedef struct 
{
	char TransportProtocol[4];	//’tcp’ if protocol is TCP, ’udp’ if protocol is UDP
	char serviceMode[7];		//’wait’ if service is concurrent, ‘nowait’ otherwise
	char port[6];			//port on which service is available
	char CompleteName[50];		//complete name of the service, including full path
	char Name[20];			//name of the service
	int SocketDescriptor;		//socket descriptor associated to the port
	int PID;			//process ID
} serviceInfo;
struct sockaddr_in server_addr;
//Function prototype devoted to handle the death of the son process
void handle_signal (int sig);

int  main(int argc,char **argv,char **env){ // NOTE: env is the variable to be passed, as last argument, to execle system-call
	// Other variables declaration goes here
	serviceInfo si[10];
	int i=0, br, lr;
	char ch;
	
	
	FILE *fileptr;
	if((fileptr = fopen(FILENAME, "r"))==NULL)
	{
			printf("errore apertura file");
			exit(1);
	};
	while(fscanf(fileptr, "%s %s %s %s\n", &si[i].CompleteName, &si[i].TransportProtocol, &si[i].port, &si[i].serviceMode)==4)
	{
	printf("%s %s %s %s\n", si[i].CompleteName, si[i].TransportProtocol, si[i].port, si[i].serviceMode);
	if (si[i].TransportProtocol=="tcp") 
	{
		si[i].SocketDescriptor = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
	}
	else
	{
		si[i].SocketDescriptor = socket(AF_INET,SOCK_DGRAM, IPPROTO_UDP);
	}
	br= bind(si[i].SocketDescriptor, INADDR_ANY
	
	if (si[i].SocketDescriptor<0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}
	i++;
	}
	// Server behavior implementation goes here
	
	
	
	signal (SIGCHLD,handle_signal); /* Handle signals sent by son processes - call this function when it's ought to be */
	
	return 0;
}

// handle_signal implementation
void handle_signal (int sig){
	// Call to wait system-call goes here
	
	switch (sig) {
		case SIGCHLD : 
			// Implementation of SIGCHLD handling goes here	
			
			
			break;
		default : printf ("Signal not known!\n");
			break;
	}
}
