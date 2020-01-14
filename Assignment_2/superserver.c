#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/time.h>
#include<netinet/in.h>
#include<signal.h>
#include<errno.h>
#include<sys/wait.h> 

#define MAX_SERVICES 10
#define MAX_CHAR 30
#define MAX_LENGTH 100
#define BACK_LOG 2

//Constants and global variable declaration goes here
int number_of_services;
int max_sfd;
int death;
int* descriptors;
fd_set readset;	

//Service structure definition goes here
typedef struct {
	char protocol[MAX_CHAR];
	char service_mode[MAX_CHAR];
	char port[MAX_CHAR];
	char full_path_name[MAX_CHAR];
	char service_name[MAX_CHAR];
    int socket_fd;
    int pid;
} service_descriptor;

service_descriptor services[MAX_SERVICES];

//Function prototype devoted to handle the death of the son process
void handle_signal (int sig);
//Get the service name from the full path of the service
char* get_service_name (char* str);
//Function that read from the file and write data into the data structure
void create_structure(FILE* f);
//Fill fd_set with socket file descrptors
void fill_select();
//Search the service executed within killed process
void searching_PID(int pid);
//Insert again socket file descriptor in FD_set
void rebuild_set();
//Remove socket file descriptor from array of descriptors
void remove_descriptor(int sfd);
//Insert socket file descriptor into array of descriptors
void insert_descriptor(int sfd);

//get_service_name implementation
char* get_service_name(char* str) {  
	char* service_name = NULL;
	char* ptr;
	
	service_name = (char*)malloc(MAX_CHAR * sizeof(char));
	if(service_name == NULL) {
		printf("Malloc error!!\n");
		exit(EXIT_FAILURE);
	}
	ptr = strrchr(str, '/');
	strncpy(service_name, ptr + 1, strlen(ptr));
	service_name[strlen(service_name)] = '\0'; 
	
	return service_name;
}

//insert_descriptor implementation
void insert_descriptor(int sfd) {
	int i;
	int found = 0;
	for (i = 0; i < number_of_services && found == 0; i++) {
		if(descriptors[i] == 0) {
			descriptors[i] = sfd;
			found = 1;
		}
	}
}
//remove_descriptor implementation
void remove_descriptor(int sfd) {
	int i;
	int found = 0;
	for (i = 0; i < number_of_services && found == 0; i++) {
		if(descriptors[i] == sfd) {
			descriptors[i] = 0;
			found = 1;
		}
	}
}

//searching_PID implementation
void searching_PID(int pid){
	int i;
	int flag = 0;
	for(i=0; i < number_of_services && flag == 0; i++){
		if(services[i].pid == pid && strcmp(services[i].service_mode, "wait") == 0){ 
			FD_SET(services[i].socket_fd, &readset);
			insert_descriptor(services[i].socket_fd);
			flag = 1;
		}
	}
}

//rebuild_set implementation
void rebuild_set(){
	int i;
	for(i = 0; i < number_of_services; i++){
		if(descriptors[i] != 0) {
			FD_SET(descriptors[i], &readset);
		}
	}
}

//create_structure implementation
void create_structure(FILE* f){
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	char* str;
	char* service_name;
	char* service_mode;
	int i = 0;
	
	while ((read = getline(&line, &len, f)) != -1 && i < MAX_SERVICES) { /* Read from the file line by line and add each data into data structure*/
        str = strtok(line, " "); 
        service_name = get_service_name(str);
        strcpy(services[i].full_path_name, str);
        strcpy(services[i].service_name, service_name);
        strcpy(services[i].protocol, strtok(NULL, " "));
        strcpy(services[i].port, strtok(NULL, " "));
        service_mode = strtok(NULL, " ");
        service_mode[strlen(service_mode) - 1] = '\0';
        strcpy(services[i].service_mode, service_mode);
        free(service_name);
		i++;
	}
	number_of_services = i;
}

//fill_select implementation
void fill_select(){
	int sfd; //socket file descriptor
	int lr; //listen result
	int br; // Bind result
	struct sockaddr_in server_addr; // struct containing server address information
	int i;
	
	descriptors = (int*)malloc(number_of_services * sizeof(int));
	if(descriptors == NULL) {
		printf("Error Malloc!");
		exit(EXIT_FAILURE);
	}
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	for(i=0; i < number_of_services; i++){ /* For each service create socket */
		server_addr.sin_port = htons(atoi(services[i].port));
		if(strcmp(services[i].protocol, "tcp") == 0){
			sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (sfd < 0){
				printf("socket error");
				exit(EXIT_FAILURE);
			}
			br = bind(sfd, (struct sockaddr *) &server_addr, sizeof(server_addr));
			if (br < 0){
				perror("bind");
				exit(EXIT_FAILURE);
			}
			lr = listen(sfd, BACK_LOG);
			if (lr < 0){
				printf("listen error");
				exit(EXIT_FAILURE);
			}
		} else {
			sfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			if (sfd < 0){
				printf("socket error");
				exit(EXIT_FAILURE);
			}
			br = bind(sfd, (struct sockaddr *) &server_addr, sizeof(server_addr));
			if (br < 0){
				perror("bind");
				exit(EXIT_FAILURE);
			}
		}
		services[i].socket_fd = sfd;
		descriptors[i] = sfd;
		FD_SET(sfd, &readset);
	}
	max_sfd = sfd;
}

// handle_signal implementation
void handle_signal (int sig){
	pid_t pid;
	// Call to wait system-call goes here
	pid = wait(NULL);
	death = 1;
	printf("Il processo killato e\': %d\n",pid);
	switch (sig) {
		case SIGCHLD : 
			// Implementation of SIGCHLD handling goes here	
			searching_PID((int)pid);
			break;
		default : printf ("Signal not known!\n");
			break;
	}
}

int main(int argc,char **argv,char **env){ // NOTE: env is the variable to be passed, as last argument, to execle system-call
	// Other variables declaration goes here
	FILE* fc;	
	pid_t pid;
	struct sockaddr_in service_addr;
	socklen_t ser_size;
	int i;
	int temp;
	int newsfd;
	int dr;
	int check;
	death = 0;
	// Server behavior implementation goes here
	fc = fopen("superserver.txt","r");
	if (fc == NULL){
		printf("Reading error!");
		exit(EXIT_FAILURE);
	}
	create_structure(fc);
	check = fclose(fc);
	if (check != 0){
		printf("Closing error!");
		exit(EXIT_FAILURE);
	}
	FD_ZERO(&readset);
	fill_select();
	ser_size = sizeof(service_addr);	
	signal (SIGCHLD, handle_signal); /* Handle signals sent by son processes - call this function when it's ought to be */
	//Infinite loop of server
	while(1){
		FD_ZERO(&readset);
		rebuild_set();
		if ((temp = select(max_sfd + 1, &readset, NULL, NULL, NULL)) < 0 && death == 0) {
			printf("Select error\n");
			exit(EXIT_FAILURE);
		} else {
			death = 0;
			for (i = 0; i < number_of_services && temp > 0; i++) {
				if(FD_ISSET(services[i].socket_fd, &readset)){
					temp--;					
					if (strcmp(services[i].protocol,"tcp") == 0){
						newsfd = accept(services[i].socket_fd, (struct sockaddr *) &service_addr, &ser_size);
					}
					if (death != 1){
						pid = fork();
						if (pid == 0){ /* Child */
							//Close all standard I/O descriptors
							close(STDIN_FILENO);
							close(STDOUT_FILENO);
							close(STDERR_FILENO);  
							//Call 3 times dup
							if (strcmp(services[i].protocol, "tcp") == 0){
								close(services[i].socket_fd);
								if(dup(newsfd) != STDIN_FILENO || dup(newsfd) != STDOUT_FILENO || dup(newsfd) != STDERR_FILENO ) {
									printf("Error duplicating socket for stdin/stdout/stderr");
									exit(EXIT_FAILURE);
								}
							} else {
								if(dup(services[i].socket_fd) != STDIN_FILENO || dup(services[i].socket_fd) != STDOUT_FILENO || dup(services[i].socket_fd) != STDERR_FILENO ) {
									printf("Error duplicating socket for stdin/stdout/stderr");
									exit(EXIT_FAILURE);
								}
							}
							check = execle(services[i].full_path_name, services[i].service_name, NULL, env); /* Performs the service */
							if (check < 0) {
								printf("Execle error!");
								exit(EXIT_FAILURE);
							}				
						} else { /* Father */
							sleep(0.7);
							if (strcmp(services[i].protocol, "tcp") == 0) {
								close(newsfd);
							}
							if (strcmp(services[i].service_mode, "wait") == 0){
								services[i].pid = pid;
								FD_CLR(services[i].socket_fd, &readset);
								remove_descriptor(services[i].socket_fd);
							}
						}
					}
				}
			}
		}
	}
	free(descriptors);
	return 0;
}
