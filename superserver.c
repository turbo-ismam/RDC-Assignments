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
	struct sockaddr_in client_addr, server_addr ; // struct containing client/server address information
	struct timeval tWait;
	fd_set active_fdset, read_fdset;
	pid_t pid;
	int i=0, br, lr, sr;
	int maxfd;
	int sock;
	char ch;
	FILE *fileptr;
		// Server behavior implementation goes here
	FD_ZERO (&active_fdset);
	FD_ZERO (&read_fdset);
	if((fileptr = fopen(FILENAME, "r"))==NULL)
	{
			printf("errore apertura file");
			exit(1);
	};
	while(fscanf(fileptr, "%s %s %s %s\n", &si[i].CompleteName, &si[i].TransportProtocol, &si[i].port, &si[i].serviceMode)==4)
	{
		
		strcpy(si[i].Name, "tcpServer.exe");
		printf("%s %s %s %s %s\n", si[i].CompleteName, si[i].Name ,si[i].TransportProtocol, si[i].port, si[i].serviceMode);
		if (si[i].TransportProtocol=="tcp")
		{
			si[i].SocketDescriptor = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
		}
		else
		{
			si[i].SocketDescriptor = socket(AF_INET,SOCK_DGRAM, IPPROTO_UDP);
		}
		printf("sfd:%d ", si[i].SocketDescriptor);
		if (si[i].SocketDescriptor<0) 
		{
			perror("socket");
			exit(EXIT_FAILURE);
		}
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(atoi(si[i].port)); // Convert to network byte order
		server_addr.sin_addr.s_addr = INADDR_ANY; // Bind to any address htonl(INADDR_ANY);
		printf("\n%d %d %d\n",server_addr.sin_family, server_addr.sin_port, server_addr.sin_addr.s_addr );
		br = bind(si[i].SocketDescriptor, (struct sockaddr *) &server_addr, sizeof(server_addr));
		if (br < 0)
		{
			perror("bind"); // Print error message
			exit(EXIT_FAILURE);
		}
		
		if (si[i].TransportProtocol=="tcp")
		{
			lr = listen(si[i].SocketDescriptor, BACK_LOG);
			if (lr < 0)
			{
				perror("listen"); // Print error message
				exit(EXIT_FAILURE);
			}
		}
		FD_SET(si[i].SocketDescriptor, &active_fdset);
		i++;
	}//end while
	maxfd=i;
	fclose(fileptr);
	signal (SIGCHLD,handle_signal); // Handle signals sent by son processes - call this function when it's ought to be 	
	for (;;)
	{
		read_fdset = active_fdset;
		tWait.tv_sec = 5;
		tWait.tv_usec = 0;
		printf("beforeSELECT\n");
		fflush(stdin);
		sr =select (maxfd+1, &read_fdset, NULL, NULL, NULL);
		printf("afterSELECT\n");
		fflush(stdin);
		if ( sr < 0)
		{
			perror("select"); // Print error message
			exit(EXIT_FAILURE);
		}
		if (sr == 0)
		{
			printf("Timeout expired, no pending connection on socket\n");
		}
		for (i = 0; i < maxfd; ++i) 
		{
			
			if (FD_ISSET (i, &read_fdset))
			{
				printf("ErrorFDISSET\n");
			}
					if (i == sock)
					{
						// Connection request on original socket. 
						int new;
						int size = sizeof (client_addr);
						new = accept (si[i].SocketDescriptor,
									  (struct sockaddr *) &client_addr,
									  &size);
						if (new < 0)
						  {
							perror ("accept");
							exit (EXIT_FAILURE);
						  }
						fprintf (stderr,
								 "Server: connect from host %s, port %hd.\n",
								 inet_ntoa (client_addr.sin_addr),
								 ntohs (client_addr.sin_port));
					}
					else
					{
						printf("ciaooo");
					}
		}	
			if (fork()==0)	//child
			{
				printf("forking...");
				close(0);
				close(1);
				close(2);
				dup(si[i].SocketDescriptor);
				dup(si[i].SocketDescriptor);
				dup(si[i].SocketDescriptor);
				execle(si[i].CompleteName, si[i].Name, argv, NULL, env);
			}
			else //parent
			{
				printf("hi!");
			}
	}
	return 0;
}

// handle_signal implementation
void handle_signal (int sig){
	printf("signal is called!");
	// Call to wait system-call goes here
	int childPID = wait(&sig);
	switch (sig) {
		case SIGCHLD :
			// Implementation of SIGCHLD handling goes here
			for(int i = 0; i < FDSIZE; i++)
			{
				printf("doing something signal--\n");
				if(si[i].PID == childPID && 1==1)
				{
					
				}
			}

			break;
		default : printf ("Signal not known!\n");
			break;
	}
}
