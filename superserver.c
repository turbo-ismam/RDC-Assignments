#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/time.h>
#include<netinet/in.h>
#include<signal.h>
#include<errno.h>

//Constants and global variable declaration goes here


//Service structure definition goes here
typedef struct 
{
	char[4] TransportProtocol;	//’tcp’ if protocol is TCP, ’udp’ if protocol is UDP
	char[7] serviceMode;		//’wait’ if service is concurrent, ‘nowait’ otherwise
	char[4] port;			//port on which service is available
	char[50] CompleteName;		//complete name of the service, including full path
	char[20] Name;			//name of the service
	int SocketDescriptor;		//socket descriptor associated to the port
	int PID;			//process ID
} serviceInfo;

//Function prototype devoted to handle the death of the son process
void handle_signal (int sig);

int  main(int argc,char **argv,char **env){ // NOTE: env is the variable to be passed, as last argument, to execle system-call
	// Other variables declaration goes here
	
	
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
