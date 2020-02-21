#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/time.h>
#include<sys/wait.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<signal.h>
#include<errno.h>
#include<unistd.h>

//Constants and global variable declaration goes here

#define BACK_LOG 2
#define FDSIZE 10
#define FILENAME "serverlist"

fd_set read_fdset;
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

serviceInfo si[FDSIZE];

//Function prototype devoted to handle the death of the son process
void handle_signal (int sig);

int  main(int argc,char **argv,char **env) // NOTE: env is the variable to be passed, as last argument, to execle system-call
{ 
	// Other variables declaration goes here
	struct sockaddr_in server_addr ; // struct containing client/server address information
	struct timeval tWait;
	pid_t pid;
	int i=0, br, lr, sr, er;
	int maxfd=0;
	int new;
	int nfd;
	int sock;
	socklen_t size;
	char ch;
	FILE *fileptr;
	
		// Server behavior implementation goes here
	FD_ZERO (&read_fdset);
	if((fileptr = fopen(FILENAME, "r"))==NULL)
	{
			printf("errore apertura file");
			exit(1);
	};
	
	while(fscanf(fileptr, "%s %s %s %s\n", si[i].CompleteName, si[i].TransportProtocol, si[i].port, si[i].serviceMode)==4)
	{
		
		strcpy(si[i].Name, si[i].CompleteName+2); //salta i primi 2 caratteri e copia il nome del servizio
		printf("%s %s %s %s %s\n", si[i].CompleteName, si[i].Name ,si[i].TransportProtocol, si[i].port, si[i].serviceMode);
		if (strcmp(si[i].TransportProtocol,"tcp")==0)
		{
			si[i].SocketDescriptor = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
		}
		else
		{
			si[i].SocketDescriptor = socket(AF_INET,SOCK_DGRAM, IPPROTO_UDP);
		}
		
		if(si[i].SocketDescriptor> maxfd)
		{
			maxfd=si[i].SocketDescriptor;
		}
		
		printf("sfd:%d ", si[i].SocketDescriptor);
		
		if (si[i].SocketDescriptor<0) 
		{
			perror("socket");
			exit(EXIT_FAILURE);
		}
		server_addr.sin_family = AF_INET;
		int atoi1 = atoi(si[i].port);
		server_addr.sin_port = htons(atoi(si[i].port)); // Convert to network byte order
		server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // Bind to any address htonl(INADDR_ANY);
		br = bind(si[i].SocketDescriptor, (struct sockaddr *) &server_addr, sizeof(server_addr));
		
		if (br < 0)
		{
			perror("bind"); // Print error message
			exit(EXIT_FAILURE);
		}
		if (strcmp(si[i].TransportProtocol,"tcp")==0)
		{
			
			lr = listen(si[i].SocketDescriptor, BACK_LOG);
			if (lr < 0)
			{
				perror("listen"); // Print error message
				exit(EXIT_FAILURE);
			}
		}
		FD_SET(si[i].SocketDescriptor, &read_fdset);
		i++;
	}//end while
	fclose(fileptr);
	signal (SIGCHLD,handle_signal); // Handle signals sent by son processes - call this function when it's ought to be 	
	nfd=i;
	for (;;)
	{
		tWait.tv_sec = 5;
		tWait.tv_usec = 0;
		sr =select (maxfd+1, &read_fdset, NULL, NULL, NULL);							//SELECT

		if ( sr < 0)
		{
			perror("select"); // Print error message
			exit(EXIT_FAILURE);
		}
		if (sr == 0)
		{
			printf("Timeout expired, no pending connection on socket\n");
		}
		for (i = 0; i < nfd; i++) 
		{
			printf("Checking socket...");
			if (!FD_ISSET (i, &read_fdset))
			{
				// Connection request on original socket. 
				if (strcmp(si[i].TransportProtocol,"tcp") == 0){
						size = sizeof(server_addr);
						new = accept (si[i].SocketDescriptor,(struct sockaddr *) &server_addr, &size);
					}
				
				if (new < 0)
				{
					perror ("accept");
					exit (EXIT_FAILURE);
				}
				pid=fork();
				if (pid==0)	//child
				{
					fprintf(stderr,"forking...");
					close(0);
					close(1);
					close(2);
					if(strcmp(si[i].TransportProtocol, "tcp")==0)
					{
						dup(new);
						dup(new);
						dup(new);
					}
					else 
					{
						dup(si[i].SocketDescriptor);
						dup(si[i].SocketDescriptor);
						dup(si[i].SocketDescriptor);
					}
					
					er = execle(si[i].CompleteName, si[i].Name, NULL, env);
					if (er < 0) {
								printf("Execle error!");
								exit(EXIT_FAILURE);
							}
				}
				else //parent
				{
					sleep(1);
					if (strcmp(si[i].TransportProtocol, "tcp") == 0) {
								close(new);
							}
							if (strcmp(si[i].serviceMode, "wait") == 0){
								si[i].PID = pid;
								FD_CLR(si[i].SocketDescriptor, &read_fdset);
								si[i].SocketDescriptor=0;
							}
				}
			}
		}	
	}
	return 0;
}

// handle_signal implementation
void handle_signal (int sig){
	printf("signal is called!");
	// Call to wait system-call goes here
	pid_t childPID = wait(&sig);
	printf("killing process:%d", childPID);
	switch (sig) {
		case SIGCHLD :
			// Implementation of SIGCHLD handling goes here
			for(int i = 0; i < FDSIZE; i++)
			{
				printf("doing something signal--\n");
				if(si[i].PID == childPID && strcmp(si[i].serviceMode, "wait")==0)
				{
					FD_SET(si[i].SocketDescriptor, &read_fdset);
					
				}
			}

			break;
		default : printf ("Signal not known!\n");
			break;
	}
}
