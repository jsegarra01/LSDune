/*
	Josep Segarra    		Josep.segarra@students.salle.url.edu
	Sergi Vives				Sergi.vives@students.salle.url.edu
*/

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
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

int socketfd = -1;
Configuration config;													//No need to protect this as it will only be
pthread_t t;
char* instructions = 0;														
char* name;
int id = 0;
int thread = 0;

void ksighandler(int signum){
	char buffer[100];
	void *res;

	switch (signum){
		case SIGINT:
			if (thread == 1) {
				pthread_cancel(t);
				pthread_join(t,&res);
			}
			if (socketfd != -1) {
				sprintf(buffer,"%s*%d", name, id);
            	createMessage("FREMEN\0", 'Q', buffer, socketfd);
				close(socketfd);
			}
			
			free(config.folder);
			free(config.ip);
			free(instructions);
			sprintf(buffer, "Closing program.\n");
        	write(STDOUT_FILENO, buffer, strlen(buffer));
			raise(SIGKILL);
			break;
	}
	signal(signum, ksighandler);
}

void* fremenFunction(){
	char source[15];
	char type;
	char data[240];
 	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	int first = 0;

	while (1) {
		if (socketfd != -1) {
			readTimes(socketfd, 15, source);
	        read(socketfd, &type, sizeof(char));
          	readTimes(socketfd, 240, data);
			write(STDOUT_FILENO, &type, 1);
			
			if (type == 'Q') {
				write(STDOUT_FILENO, data, strlen(data));
				close(socketfd);
				socketfd = -1;
				id = 0;
			}
		}
		else if (first == 0) {
			first = 1;
			sprintf(data, "socketfd is in thread: %d\n", socketfd);
			write(STDOUT_FILENO, data, strlen(data));
		}
	}
}

int main (int argc, char *argv[]) {
	char buffer[256];
	u_int16_t port;
	struct in_addr ip_addr;
	char string[240];
	char source[15];
	char type;
	char data[240];
 	int counter = 0, space = 0, aux = 0;
 	char character;
	char* aux3;
	char* aux2;
	char* token;
	char md5sum[236];
    struct stat fileSearch;
	void* res;
 	pid_t child;
    int wstatus;

    
	if (argc != 2) {
		sprintf(buffer, "There has been a problem with the number of arguments\n");
	    write(STDOUT_FILENO, buffer, strlen(buffer));
	}
	else if (readConfigFile(&config,argv[1])) {									//Reads the file, returns 1 if error
		sprintf(buffer, "There has been a problem opening the file %s\n", argv[1]);
        write(STDOUT_FILENO, buffer, strlen(buffer));
	}
    else {
		sprintf(buffer, "Welcome to Fremen\n\n");
		write(STDOUT_FILENO, buffer, strlen(buffer));
    	
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
		
		//clean the folder from the first /
		if (config.folder[0] == '/') {
            for (int i = 0; i < (int)strlen(config.folder) - 1; i++) {
                config.folder[i] = config.folder[i+1];
            }
            config.folder[strlen(config.folder) - 1] = '\0';
        }

		signal(SIGINT, ksighandler);

   	 	do {                                //Reads the functions the user is writing as commands
        	aux = 0;
        	space = 0;
        	instructions = (char*) malloc(sizeof(char));
			
			thread = 1;
			pthread_create(&t,NULL,fremenFunction,NULL);
		
			read(STDIN_FILENO,&character,1);       
       		counter = 0;

        	thread = 0;
			pthread_cancel(t);
			pthread_join(t, &res);
			
			sprintf(buffer, "socketfd: %d\n", socketfd);
			write(STDOUT_FILENO,buffer,strlen(buffer));


			while (character != '\n'){
            	if (space < 1) {							             
					if (character == ' ') {
						instructions[counter] = '\0';
						space++;
					}
					else {
						instructions[counter] = character;
						instructions = (char*) realloc(instructions,sizeof(char)*(counter + 2));
					}
				}
				if (character == ' ') {
					character = '*'; 
				}	
            	string[counter] = character;
				counter++;
            	read(STDIN_FILENO,&character,1);
       	 	}
            
			string[counter] = '\0';

			while(counter < 240) {
				string[++counter] = '\0';
			}

        	
			/*if (strlen(instructions) == strlen(string)) {
				 aux = 1;
				 if (instructions[counter - 1] == ' '){
	                 instructions[counter - 1] = '\0';
    	         }
			}
			else {
				instructions[strlen(instructions) - 1] = '\0';
			}*/
			

			convertLowerCase(instructions);


			if (!strcmp(instructions, "login")) {
				if (aux) {
					sprintf(buffer,"You need to introduce parameters to the command\n\n");
					write(STDOUT_FILENO,buffer,strlen(buffer));
				}
				else if (socketfd == -1){			
					socketfd = connectToServer(ip_addr, port);
					
					if (socketfd == -1) {
						sprintf(buffer, "There has been a problem connecting to the server\n");
						write(STDOUT_FILENO, buffer, strlen(buffer));
					}
					else {
						createMessage("FREMEN\0", 'C', string, socketfd);
					
						readTimes(socketfd, 15, source);	
						read(socketfd, &type, sizeof(char));
						readTimes(socketfd, 240, data);

						if (type == 'T') {								//This was E, I don't know why	
							sprintf(buffer,"There has been an error logging in\n\n");
 							write(STDOUT_FILENO,buffer,strlen(buffer));
						}
						else {
							id = convertToInt(data);

							token = strtok(string, "*");
							if(strcmp(token,"HOLA")) {
								name = "\0";
							}
							name = strtok(NULL, "*");
							sprintf(buffer, "Welcome %s. You have the ID %d\n", name, id);
         					write(STDOUT_FILENO,buffer,strlen(buffer));
					
           					sprintf(buffer,"You are now connected to Atreides.\n\n");
           					write(STDOUT_FILENO,buffer,strlen(buffer));
       					}
					}
				}
				else {
					 sprintf(buffer,"You have already logged in!\n\n");
                     write(STDOUT_FILENO,buffer,strlen(buffer));
				}
			} else if (!strcmp(instructions, "search")) {
           		if (aux) {
					sprintf(buffer,"You need to introduce parameters to the command\n\n");
 					write(STDOUT_FILENO,buffer,strlen(buffer));
				}
 				else if (socketfd == -1){
					sprintf(buffer,"You need to log in first!");
					write(STDOUT_FILENO,buffer,strlen(buffer));
				}
				else {
					createMessage("FREMEN\0", 'S', string, socketfd);
			
					readTimes(socketfd, 15, source);	
					read(socketfd, &type, sizeof(char));
					readTimes(socketfd, 240, data);

					if (type == 'T') {										//This was K I don't know why
						sprintf(buffer, "\nThere has been an error in the transmission\n");
						write(STDOUT_FILENO,buffer,strlen(buffer));
					}
					else {
						aux2 = strtok(data, "*");
						int aux = convertToInt(aux2);
					
						if (aux == 0) {
							sprintf(buffer,"There are 0 human beings\n");
							write(STDOUT_FILENO,buffer,strlen(buffer));
						}
						else {
							if (aux == 1) {
								sprintf(buffer,"There is 1 human being\n");
							}
							else {
								sprintf(buffer,"There are %d human beings\n", aux);
							}
 							write(STDOUT_FILENO,buffer,strlen(buffer));

							int another = 1;
							int counterPeople = 0;
							int people = countPeople(data) + 1;
		
							while(another) {
								another = 0;

								for (int j = 0; j < ((people/2 ) -1); j++) {
									aux2 = strtok(NULL, "*");
									aux3 = strtok(NULL, "*");
									sprintf(buffer, "%s %s\n", aux3, aux2);
									write(STDOUT_FILENO,buffer,strlen(buffer));
								}
								aux2 = strtok(NULL, "*");
								aux3 = strtok(NULL, "\0");
								sprintf(buffer, "%s %s\n", aux3, aux2);
								write(STDOUT_FILENO,buffer,strlen(buffer));	
								
								if ((people + counterPeople) != aux*2) {
									counterPeople += people;
									another = 1;							
									readTimes(socketfd, 15, source);	
									read(socketfd, &type, sizeof(char));
									readTimes(socketfd, 240, data);
							
									people = countPeople(data);

									aux2 = strtok(data, "*");
									aux3 = strtok(NULL, "*");
									sprintf(buffer, "%s %s\n", aux3, aux2);
									write(STDOUT_FILENO,buffer,strlen(buffer));
							
									people -= 2;
									counterPeople += 2;

									if(people == 0) {
										another = 0;
									}
								}
							}
						}
					}			
				}
			} else if (!strcmp(instructions, "send")) {
				if (aux){
					sprintf(buffer,"You need to introduce parameters to the command\n\n");
                    write(STDOUT_FILENO,buffer,strlen(buffer));
				}
				else if (socketfd == -1) {
           			sprintf(buffer,"You need to log in first!\n\n");
           			write(STDOUT_FILENO,buffer,strlen(buffer));
       			}
				else {
					aux2 = strtok(string, "*");
					aux2 = strtok(NULL,"*");
					sprintf(buffer, "%s/%s", config.folder, aux2);
					
					aux = stat(buffer, &fileSearch);
					if(aux == 0) {												//Checks if file exists
						aux3 = convertToPoint(fileSearch.st_size);
 						sprintf(data,"%s", aux3);
						write(STDOUT_FILENO,data,strlen(data));
						getMd5sum(buffer, md5sum);
						sprintf(data,"%s*%s*%s", aux2, aux3, md5sum);
						free(aux3);
											
						int imagefd = open(buffer, O_RDONLY);
						if (imagefd < 0) {
							sprintf(data, "It was not possible to send the file\n\n");
						}
						else {
							createMessage("FREMEN\0", 'F', data, socketfd);         //This is the first frame for the image
							readImage(fileSearch.st_size, imagefd, socketfd);
							close(imagefd);
							readTimes(socketfd, 15, source);	
							read(socketfd, &type, sizeof(char));
							readTimes(socketfd, 240, data);
						}
						write(STDOUT_FILENO,data,strlen(data));
					}
					else {
						sprintf(buffer,"No file was found with that name\n\n"); 
						write(STDOUT_FILENO,buffer,strlen(buffer));
					}
				}
			} else if (!strcmp(instructions, "photo")) {
            	if (aux) {
					 sprintf(buffer,"You need to introduce parameters to the command\n\n");
                     write(STDOUT_FILENO,buffer,strlen(buffer));
				}
				else if (socketfd == -1) {
					sprintf(buffer,"You need to log in first!\n\n");
            		write(STDOUT_FILENO,buffer,strlen(buffer));
        		}
				else {
					aux2 = strtok(string, "*");
					aux2 = strtok(NULL, "*");
					sprintf(buffer, "%s", aux2);
					createMessage("FREMEN\0", 'P', buffer, socketfd);
					
					readTimes(socketfd, 15, source);
					read(socketfd, &type, sizeof(char));
					readTimes(socketfd, 240, data);

					if (strcmp(data,"FILE NOT FOUND") == 0) {
						sprintf(buffer, "Photo not registered\n\n");
						write(STDOUT_FILENO, buffer, strlen(buffer));
					}
					else {
						aux2 = strtok(data,"*");
						sprintf(buffer, "%s/%s.jpg", config.folder, aux2);
						aux2 = strtok(NULL, "*");
						aux = convertToInt(aux2);
						aux3 = strtok(NULL, "*");
						
						sprintf(data, "%d\n", aux);
						write(STDOUT_FILENO, data, strlen(data));
						unlink(buffer);
						int imagefd = open(buffer, O_WRONLY | O_APPEND | O_CREAT | O_TRUNC, 0x777); 
						if (imagefd < 0) {
 							sprintf(data, "It was not possible to open the file\n\n");
 						} 
						else {
							writeImage(aux, imagefd, socketfd);     //It reads it in binary
							close (imagefd);
							getMd5sum(buffer,md5sum);
							
							sprintf(buffer, "\nThis md5sum %s END\n", md5sum);
							write(STDOUT_FILENO, buffer, strlen(buffer));

							if (!strcmp(aux3, md5sum)) {
								createMessage("ATREIDES\0", 'I', "IMAGE OK\0", socketfd);
								sprintf(buffer, "Photo downloaded!\n");
								write(STDOUT_FILENO, buffer, strlen(buffer));
							}
							else {
								createMessage("ATREIDES\0", 'R', "IMAGE KO\0", socketfd);
								//unlink(buffer);
								sprintf(buffer, "The photo was not downloaded!\n");
								write(STDOUT_FILENO, buffer, strlen(buffer));
							}
						}
					}
				}
			} else if (!strcmp(instructions, "logout")) {
				if (socketfd != -1) {
					
					sprintf(buffer,"%s*%d", name, id);
					createMessage("FREMEN\0", 'Q', buffer, socketfd);
					close(socketfd);

					socketfd = -1;
					id = 0;
					sprintf(buffer,"logout!\n\n");
            		write(STDOUT_FILENO,buffer,strlen(buffer));	
				}
				else {
					sprintf(buffer,"You have not logged in! %d\n\n", id);					//Todo delete the id from here
 					write(STDOUT_FILENO,buffer,strlen(buffer));
				}
			} else {
			    
			    child = spawnChild(instructions, parseString(string));

                //if (waitpid(child, &wstatus, WUNTRACED | WCONTINUED) == -1) {
                if (waitpid(child, &wstatus, WUNTRACED) == -1) {
					 sprintf(buffer,"There has been a problem executing the command");
					 write(STDOUT_FILENO,buffer,strlen(buffer));
                }
                
       		}

        	free(instructions);
   	 	} while (1);

		free(config.ip);
		free(config.folder);
	}
	exit(0);
}
