/*
	Josep Segarra    		Josep.segarra@students.salle.url.edu
	Sergi Vives			Sergi.vives@students.salle.url.edu
*/
// valgrind --track-origins=yes --leak-check=full --track-fds=yes --show-reachable=yes 

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>
#include <sys/shm.h>
#include "utilities.h"
#include "cmd.h"
#include "connectivity.h"
#include "semaphore.h"

typedef struct {
	int socketfd;
	int id;
} DedServers;

ConfigurationAtreides config;                                          
pthread_t* t;															
DedServers* server;														
int counterLogins = 1;

semaphore newThread;											//Protects the thread creation, so they don't overwrite themselves
pthread_mutex_t memoryMutex = PTHREAD_MUTEX_INITIALIZER;		//Protects the access to the memory so threads can't write or read at the same time
pthread_mutex_t imageMutex = PTHREAD_MUTEX_INITIALIZER;


void ksighandler(int signum){
	char buffer[100];
	void* res;

    switch (signum){
        case SIGINT:
			pthread_mutex_lock(&memoryMutex);
			pthread_mutex_lock(&imageMutex);

			counterLogins--;
			while (counterLogins >= 0) {
				if (server[counterLogins].socketfd != -1) {
					if (counterLogins > 0) {
						pthread_cancel(t[counterLogins - 1]);
						//createMessage("ATREIDES\0", 'Q', "The server ATREIDES has closed down\n", server[counterLogins].socketfd);
					}
					close(server[counterLogins].socketfd);
				}
				if (counterLogins > 0) {
					pthread_join (t[counterLogins - 1], &res);
				}
				counterLogins--;
			}
			free(server);
			free(t);
			free(config.ip);
			free(config.folder);

			pthread_mutex_unlock(&imageMutex);
			pthread_mutex_destroy(&imageMutex);
			pthread_mutex_unlock(&memoryMutex);
			pthread_mutex_destroy(&memoryMutex);
            SEM_destructor(&newThread);
			sprintf(buffer, "Closing program.\n");
            write(STDOUT_FILENO, buffer, strlen(buffer));
            raise(SIGKILL);
            break;
    }
    signal(signum, ksighandler);
}


void* atreidesFunction(void* voidPoint) {
	int *other  = (int*) voidPoint;
	int index = *other;
	int socketfd = server[index].socketfd;
	int id = server[index].id;
	int aux = 0;
	int aux4 = 0;
	char source[15];
	char type;
	char command[240];
	char msg[240];
	char buffer[300];
	char md5sum[236];
	char* aux2;
	char* aux3;
	Search* search;
	int exit = 1;
	char nameImage[256];
	sprintf(nameImage,"%s/%d.",config.folder, server[index].id);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);


	SEM_signal(&newThread);
	while (exit && socketfd != -1) {													//TODO when logout received, exit the loop and close connection
		sprintf(buffer, "Waiting connections...\n\n");
		write(STDOUT_FILENO, buffer, strlen(buffer));

		readTimes(socketfd, 15, source);	
		read(socketfd, &type, sizeof(char));
		readTimes(socketfd, 240, command);	

		if(strcmp(source, "FREMEN") != 0 || (type != 'Q' && type != 'S' && type != 'F' && type != 'D' && type != 'P' && type != 'I' && type != 'R' && type != 'C')) {
			createMessage("ATREIDES\0", 'K', "0", socketfd);
		}
		else {
			aux3 = strtok(command, "*");
			convertLowerCase(command);

			if (type == 'S') {
				aux2 = strtok(NULL, "*");
				aux4 = convertToInt(aux2);

				sprintf(buffer, "Received search %d from %d\n", aux4, id);	//TODO write name
				write(STDOUT_FILENO, buffer, strlen(buffer));

				pthread_mutex_lock(&memoryMutex);									//Protects the memory
				search = readMainMemorySearch(&aux, aux4, config.folder);
				pthread_mutex_unlock(&memoryMutex); 

				sprintf(buffer, "Search finished\nThere are %d human beings at %d:\n", aux, aux4);
				write(STDOUT_FILENO, buffer, strlen(buffer));
			
				if (aux != -1) {									//There was a problem reading the server
					aux3 = convertToPoint(aux);

					sprintf(msg,"%s",aux3);
					free(aux3);

					int another = 1;
					int counterAux = 0;

					while(another) {
						another = 0;
						for(int i = counterAux; i < aux; i++) {

							if((strlen(msg) + strlen(search[i].name) + strlen(search[i].id) + 3) >= 240) {
								another = 1;
								counterAux = i;
								break;
							}	
							else {
								strcat(msg,"*");
								strcat(msg,search[i].name);
								strcat(msg,"*");
								strcat(msg,search[i].id);
								sprintf(buffer, "%s %s\n", search[i].id, search[i].name);
								write(STDOUT_FILENO, buffer, strlen(buffer));
								free(search[i].name);
								free(search[i].id);
							}					
						}

						strcat(msg, "\0");
						createMessage("ATREIDES\0", 'L', msg, socketfd);	
						for (int i = 0; i < 240; i++) {
							msg[i] = '\0';
						}
										
					}            	
				}
				else {
					createMessage("ATREIDES\0", 'Z', "ERROR T\0", socketfd);
				}
				free(search);
			}
			else if (type == 'F') {							//Saves the picture
				aux2 = strtok(NULL, "*");					//Size of the picture
				char* md5 = strtok(NULL, "*");					//md5sum

				sprintf(buffer, "Received send %s from %d\n", aux3, id); 	//TODO we need to put the name
				write(STDOUT_FILENO, buffer, strlen(buffer));


				aux3 = strtok(aux3, ".");
				aux3 = strtok(NULL, ".");
				strcat(nameImage, aux3);
				
				aux = convertToInt(aux2);

				pthread_mutex_lock(&imageMutex);
				unlink(nameImage);
				int imagefd = open (nameImage, O_WRONLY | O_APPEND | O_CREAT | O_TRUNC, 0x777);
				
				if (imagefd < 0) {
					sprintf(buffer,"There was a problem opening the file\n");
					write(STDOUT_FILENO,buffer,strlen(buffer));
				}
				else {
					writeImage(aux, imagefd, socketfd);		//It reads it in binary	
					close (imagefd);
				}
				
				getMd5sum(nameImage, md5sum);

				if(!strcmp(md5, md5sum)) {
					createMessage("ATREIDES\0", 'I', "IMAGE OK\0", socketfd);
					sprintf(buffer, "Stored as %d.%s\n", id,aux3);
					write(STDOUT_FILENO, buffer, strlen(buffer));
				}
				else {
					createMessage("ATREIDES\0", 'R', "IMAGE KO\0", socketfd);
					unlink(nameImage);
				}
				pthread_mutex_unlock(&imageMutex);

				aux2 = strtok(nameImage, ".");
				//strcpy(nameImage,aux2);
				strcat(nameImage, ".");

			}
			else if (type == 'P') {
				struct stat fileSearch;
				sprintf(buffer, "Received photo %s from %d\n", command, id);		//Todo introduce name
				write(STDOUT_FILENO, buffer, strlen(buffer));
				aux4 = convertToInt(command);


 				sprintf(command, "%s/%d.jpg", config.folder, aux4);//TODO do they introduce .jpg or just     the id?
				aux = stat(command, &fileSearch);
				if (aux == 0) {
					aux2 = convertToPoint(fileSearch.st_size);
					getMd5sum(command,md5sum);
					sprintf(msg, "%d*%s*%s", aux4, aux2, md5sum);
				    free(aux2);

					pthread_mutex_lock(&imageMutex);
					int imagefd = open(command, O_RDONLY);
                    if (imagefd < 0) {
                        sprintf(buffer, "It was not possible to send the file\n\n");
                    	pthread_mutex_unlock(&imageMutex);
					}
                    else {
						createMessage("ATREIDES\0", 'F', msg, socketfd);
						sprintf(buffer, "Sending %d.jpg, size: %ld\n", aux4, fileSearch.st_size);
						write(STDOUT_FILENO, buffer, strlen(buffer));

						readImage(fileSearch.st_size, imagefd, socketfd);
						close(imagefd);
						pthread_mutex_unlock(&imageMutex);
						
						readTimes(socketfd, 15, source);
 						read(socketfd, &type, sizeof(char));
						readTimes(socketfd, 240, command);
						
						sprintf(buffer, "Reply sent\n");
					}
					write(STDOUT_FILENO, buffer, strlen(buffer));
				}
				else {
					createMessage("ATREIDES\0", 'F', "FILE NOT FOUND\0", socketfd);
				}
			}
			else if (type == 'Q') {
				sprintf(buffer, "Received logout from %d\nDisconnected from Atreides.\nWaiting connections...\n\n", id);	//TODO put name
				write(STDOUT_FILENO, buffer, strlen(buffer));
				close(socketfd);
				server[index].socketfd = -1;
				exit = 0;
			}
		}
	}		//end of the while
	return 0;
}

void listeningMenu(){
	char buffer[300];
	char source[15];
	char type;
	char command[240];
	char* aux2;
	char* aux3;
	char* token;
	int aux = 0;
	int* other;
	
	while(1) {
        struct sockaddr_in c_addr;
		socklen_t c_len = sizeof (c_addr);
		
		// When executing accept we should add the same cast used in the bind function
        int newsock = accept (server[0].socketfd, (void *) &c_addr, &c_len);
		
		if (newsock < 0){
            sprintf(buffer, "There was an error listening\n");
	        write(STDOUT_FILENO, buffer, strlen(buffer));
            exit (EXIT_FAILURE);
        }
		
		//sprintf(buffer, "New connection from %s:%hu\n",inet_ntoa (c_addr.sin_addr), ntohs (c_addr.sin_port));
		//write(STDOUT_FILENO, buffer, strlen(buffer));

		readTimes(newsock, 15, source);			
        read(newsock, &type, sizeof(char));
		readTimes(newsock, 240,command);

		if(strcmp(source, "FREMEN") != 0 || (type != 'Q' && type != 'S' && type != 'F' && type != 'D' && type != 'P' && type != 'I' && type != 'R' && type != 'C')) {
			createMessage("ATREIDES\0", 'E', "ERROR", newsock);
		}
		else {
			token = strtok(command, "*");
			if(strcmp(token, "hola")){
				aux2 = aux3;
			}
			convertLowerCase(command);

			if (type == 'C') {
				aux2 = strtok(NULL, "*");
				aux3 = strtok(NULL, "*");
				
				pthread_mutex_lock(&memoryMutex);									//Protects the mainMemory when accessing it
				aux = readMainMemoryName(aux2, aux3, config.folder);				//Obtain the ID and save the user if it is not in the memory
				pthread_mutex_unlock(&memoryMutex); 

				sprintf(buffer, "Received %s %s %s\nAssigned to id %d\n", command, aux2, aux3, aux);
				write(STDOUT_FILENO, buffer, strlen(buffer));
				
			
				aux3 = convertToPoint(aux);
				createMessage("ATREIDES\0", 'O', aux3, newsock);				

				other = &counterLogins;
				t = (pthread_t*)realloc(t, sizeof(pthread_t)*(counterLogins));
				pthread_create (&t[counterLogins - 1], NULL, atreidesFunction, other);
				//t = (pthread_t*)realloc(t, sizeof(pthread_t)*(counterLogins + 1));
				server = (DedServers*)realloc(server,sizeof(DedServers)*(counterLogins + 1));			
				
				server[counterLogins].socketfd = newsock;
				server[counterLogins].id = aux;

				SEM_wait(&newThread);
				counterLogins++;
				
				free(aux3);
			}
			//free(command);
		}		
    }
}

int main (int argc, char *argv[]) {
	uint16_t port;
    int aux = 0;
	struct in_addr ip_addr;
    int socketfd;
    char buffer[100];

	sprintf(buffer, "ATREIDES SERVER\n");
 	write(STDOUT_FILENO, buffer, strlen(buffer));


	if (argc != 2) {
		sprintf(buffer, "There has been a problem with the number of arugments\n");
	    write(STDOUT_FILENO, buffer, strlen(buffer));
	}
	else if (readConfigFileAtreides(&config,argv[1])) {									//Reads the file, returns 1 if error
		sprintf(buffer, "There has been a problem opening the file %s\n", argv[1]);
        write(STDOUT_FILENO, buffer, strlen(buffer));
	}
    else {
		signal(SIGINT, ksighandler);
		
		// Check if the port is valid
        aux = config.port;
        if (aux < 1 || aux > 65535){
            sprintf(buffer, "Error: %s is an invalid port\n", argv[2]);
            write(STDOUT_FILENO, buffer, strlen(buffer));
            exit (EXIT_FAILURE);
        }
        port = aux;

        // Check if the IP is valid and convert it to binary format
        if (inet_aton (config.ip, &ip_addr) == 0){
            sprintf(buffer, "Error: %s is an invalid ip\n", argv[1]);
            write(STDOUT_FILENO, buffer, strlen(buffer));
            exit (EXIT_FAILURE);
        }

		if (config.folder[0] == '/') {
			for (int i = 0; i < (int)strlen(config.folder) - 1; i++) {
				config.folder[i] = config.folder[i+1];
			}
			config.folder[strlen(config.folder) - 1] = '\0';
		}

		sprintf(buffer, "Configuration file read\nWaiting connections\n\n");
 		write(STDOUT_FILENO, buffer, strlen(buffer));
		socketfd = connectToClient(port);
		
		SEM_constructor(&newThread);										//Initializes semaphores and mutex
		SEM_init(&newThread,0);
		pthread_mutex_init(&memoryMutex, NULL);
		pthread_mutex_init(&imageMutex, NULL);

		t = (pthread_t*)malloc(sizeof(pthread_t));
		server = (DedServers*)malloc(sizeof(DedServers)*2);
		
		server[0].socketfd = socketfd;
		server[0].id = 0;

		listeningMenu();
	}
	exit(0);
}
