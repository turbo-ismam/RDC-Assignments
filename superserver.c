#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/time.h>
#include<netinet/in.h>
#include<signal.h>
#include<errno.h>
#include<unistd.h>
#define BACK_LOG 2
#define FILENAME "serverlist"
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

//Function prototype devoted to handle the death of the son process
void handle_signal (int sig);

int  main(int argc,char **argv,char **env){ // NOTE: env is the variable to be passed, as last argument, to execle system-call
	// Other variables declaration goes here
	serviceInfo si[10];
	struct sockaddr_in server_addr[10];
	fd_set fdset;
	pid_t pid;
	int i=0, br, lr;
	char ch;
	FILE *fileptr;
		// Server behavior implementation goes here
	FD_ZERO (&fdset);
	if((fileptr = fopen(FILENAME, "r"))==NULL)
	{
			printf("errore apertura file");
			exit(1);
	};
	while(fscanf(fileptr, "%s %s %s %s\n", &si[i].CompleteName, &si[i].TransportProtocol, &si[i].port, &si[i].serviceMode)==4)
	{

		printf("%s %s %s %s\n", si[i].CompleteName, si[i].TransportProtocol, si[i].port, si[i].serviceMode);
		server_addr[i].sin_family = AF_INET;
		server_addr[i].sin_port = htons(atoi(si[i].port)); // Convert to network byte order
		server_addr[i].sin_addr.s_addr = INADDR_ANY; // Bind to any address
		if (si[i].TransportProtocol=="tcp")
		{
			si[i].SocketDescriptor = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
		}
		else
		{
			si[i].SocketDescriptor = socket(AF_INET,SOCK_DGRAM, IPPROTO_UDP);
		}

		if (si[i].SocketDescriptor<0) 
		{
			perror("socket");
			exit(EXIT_FAILURE);
		}
		br = bind(si[i].SocketDescriptor, (struct sockaddr *) &server_addr[i], sizeof(server_addr[i]));
		
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
			FD_SET(si[i].SocketDescriptor, &fdset);
			/*
			sr =select (FD_SETSIZE, &fdset, NULL, NULL, NULL)
			for (i = 0; i < FD_SETSIZE; ++i)
			{
				if (FD_ISSET (i, &read_fd_set))
				{
					if (i == sock)
					{
						/* Connection request on original socket. 
						int new;
						size = sizeof (clientname);
						new = accept (sock,
									  (struct sockaddr *) &clientname,
									  &size);
						if (new < 0)
						  {
							perror ("accept");
							exit (EXIT_FAILURE);
						  }
						fprintf (stderr,
								 "Server: connect from host %s, port %hd.\n",
								 inet_ntoa (clientname.sin_addr),
								 ntohs (clientname.sin_port));
						FD_SET (new, &active_fd_set);
					}
					else
					{
						/* Data arriving on an already-connected socket. 
						if (read_from_client (i) < 0)
						{
							close (i);
							FD_CLR (i, &fdset);
						}
					}
				}
			}
			if ( sr < 0)
			{
				perror("listen"); // Print error message
				exit(EXIT_FAILURE);
			}
        {
          perror ("select");
          exit (EXIT_FAILURE);
        }*/

		signal (SIGCHLD,handle_signal); /* Handle signals sent by son processes - call this function when it's ought to be */
		}		
			if (fork()==0)
			{
				close(0);
				close(1);
				close(2);
				dup(si[i].SocketDescriptor);
				dup(si[i].SocketDescriptor);
				dup(si[i].SocketDescriptor);
				execle(si[i].CompleteName, argc, argv, NULL, env);
			}
			else
			{
				
			}
			i++;
	}
	sleep(10);
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
