/*
	Josep Segarra    		Josep.segarra@students.salle.url.edu
	Sergi Vives			Sergi.vives@students.salle.url.edu
*/
#ifndef _CONNECTIVITY_H_
#define _CONNECTIVITY_H_

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
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

#define ARG_COUNT 3
#define ERR_ARG "Invalid number of arguments. %d argument have been received.\n"
#define EOL "\n"
#define ERR_SOCKET_CREATION "Couldn't connect the socket\n"
#define ERR_CONNECT "Connection error: errno says: %s\n"
#define SOCKET_CONNECT "Connected to server\n"
#define INPUT "Enter a name: "
#define CLOSING_CONNECTION "\nClosing connection...\n"


/*
* Connects to the server, to be used by the Fremen
*/
int connectToServer(struct in_addr ip, int port);

/*
* Connects to the client, to be used by Atreides
*/
int connectToClient(int port); 

/*
* Sends the input through the socket
*/
void sendSocket(char* input, int socketfd);


int countPeople(char* msg);


/*
* Reads from the socket, until cFi is found, usually '\0'
*/
char* readUntil(int fd, char cFi);


void readTimes(int fd, int times, char*msg);


void readFrame(int fd, int times, char* frame);


void createMessage(char* source, char type, char* data, int fd);


/*
*	Gets the direct response from the socket
*/
void getResponseDirectSocket(int sockfd);

/*
* Gets a response from the socket with a modified version in front of it
*/
void getResponseSocket(char* phrase, int sockfd);

void readImage(int aux, int imagefd, int socketfd);

void writeImage(int aux, int imagefd, int socketfd);


#endif
