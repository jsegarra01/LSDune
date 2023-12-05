/*
	Josep Segarra    		Josep.segarra@students.salle.url.edu
	Sergi Vives				Sergi.vives@students.salle.url.edu
*/

#include "connectivity.h"


/*
* connects to the server, to be used by Fremen
*/
int connectToServer(struct in_addr ip, int port){
  	int socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  	//Checking for socket creation
  	if (socketfd < 0) {
    	return -1;
  	}

  	//Creating the address struct to connect
  	struct sockaddr_in socket_addr;
  	memset(&socket_addr, 0, sizeof(socket_addr));
  	socket_addr.sin_family = AF_INET;
  	socket_addr.sin_port = htons(port);
  	socket_addr.sin_addr = ip;

  	//Getting response to connection
  	int resp = connect(socketfd, (struct sockaddr *) &socket_addr, sizeof(socket_addr));

  	//Checking connection
  	if (resp < 0) {
    	close(socketfd);
    	return -1;
  	}
	
	return socketfd;
}

/*
* Connects to the client, to be used by Atreides
*/
int connectToClient(int port){ //(struct in_addr ip, int port){
   int socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
   struct sockaddr_in s_addr;
   char buffer[100];
   //Checking for socket creation
   if (socketfd < 0) {
      write(STDOUT_FILENO, ERR_SOCKET_CREATION, strlen(ERR_SOCKET_CREATION));
      exit(EXIT_FAILURE);
   }

   // Specify the adress and port of the socket
   // We'll admit connexions to any IP of our machine in the specified port
   bzero (&s_addr, sizeof (s_addr));
   s_addr.sin_family = AF_INET;
   s_addr.sin_port = htons (port);
   s_addr.sin_addr.s_addr = INADDR_ANY;

   // When executing bind, we should add a cast:
   // bind waits for a struct sockaddr* and we are passing a struct sockaddr_in*
   if (bind (socketfd, (void *) &s_addr, sizeof (s_addr)) < 0){
   		 sprintf(buffer, "There has been a problem binding the server\n");
		 write(STDOUT_FILENO, buffer, strlen(buffer));
         close(socketfd);
		 exit (EXIT_FAILURE);
   }

   // We now open the port (5 backlog queue, typical value)
   listen (socketfd, 5);
   return socketfd;
}

/*
* Sends information through the socket
*/
void sendSocket(char* input, int socketfd){
    send(socketfd, input, strlen(input)+1, 0);
	//write(socketfd,input,strlen(input)+1);
}

int countPeople(char* msg) {
	int counter = 0;

	for(int i = 0; i < 240; i ++) {
		if(msg[i] == '*') {
			counter++;
		}
	}

	return counter;
}


void createMessage(char* source, char type, char* data, int fd) {
	int counter = 0;
	int counter2 = 0;
	char* msg = (char*)malloc(sizeof(char)*256);
	
	do {
		counter = 0;
		while(counter < 15 && source[counter] != '\0') {
			msg[counter] = source[counter];
			counter++;
		}

		while(counter < 15) {
			msg[counter] = '\0';
			counter++;
		}
	
		msg[counter] = type;
		counter++;

		while(counter < 256 && data[counter2] != '\0') {
			msg[counter] = data[counter2];
			counter++;
			counter2++;
		}

		while(counter < 256) {
			msg[counter] = '\0';
			counter++;
			counter2++;
		}
		write(fd,msg, sizeof(char)*256);
	} while (data[counter2]!= '\0' && counter2 < (int) strlen(data));
	
	free(msg);
}


void readTimes(int fd, int times, char* msg) {
	//char* aux;
	//int counter = 1;
	int counter = 0;
	//read(fd, &aux,1);										//We keep it waiting here, before creating the malloc so we won't have to free it later.
															//If we do a ctrl + c 
	//msg[0] = *aux;
	while (counter < times) {
		read(fd, &msg[counter], 1);
		counter++;
	}
}

void readFrame(int fd, int times, char* frame) {
	int counter = 0;
	while (counter < times) {
		read(fd, &frame[counter], 1);
		counter++;
	}
}


/*
* Reads the socket until a certain character has been found, usually '\0'
*/
char* readUntil(int fd, char cFi) {
    int i = 0;
    char c = '0';
    char* buffer = (char*)malloc(sizeof(char));
//	char memory[200];
    
	while (c != cFi) {
        read(fd, &c, sizeof(char));
//		sprintf(memory,"char = %c\n", c);
//		write(STDOUT_FILENO, memory, strlen(memory));

        if (c != cFi) {
            buffer[i] = c;
            buffer = (char*)realloc(buffer, sizeof(char) * (i + 3));
        }
        i++;
    }
    buffer[i - 1] = '\0';
    return buffer;
}

/*
* Get a response from the socket
*/
void getResponseDirectSocket(int sockfd){
	char buffer[242];
    char* id;
	char* name;
    
    name = readUntil(sockfd, '*');
	id = readUntil(sockfd, '*');
	sprintf(buffer, "%s %s\n", id, name);
    write(STDOUT_FILENO,buffer,strlen(buffer));
	
	free(id);
	free(name);
}

void getResponseSocket(char* phrase ,int sockfd) {
 	char* response;
	char buffer[200];
 	
	
	strcat(buffer,phrase);
	response = readUntil(sockfd, '\0');
	strcat(buffer,response);
 	strcat(buffer,"\n");
	write(STDOUT_FILENO,buffer,strlen(buffer));
 	
	free(response);
}

void readImage(int aux, int imagefd, int socketfd) {
	unsigned char type;
	char binary;
	char data[240];
	int counter = 0;
	

    for (int k = 239; k >= 0; k--) {
        data[k] = '\0';
    }

	while (aux > 0) {
		read(imagefd,&type,sizeof(char));

		for (int j = 7; j >= 0;j--) {
			binary = type%2 + '0';
			type = type/2;
			data[counter + j] = binary;
		}

		aux--;
		counter = counter+8;
		if (counter + 8 == 240 || aux == 0) {
			createMessage("FREMEN\0", 'D', data, socketfd);
			counter = 0;
			for (int k = 239; k >= 0; k--) {
				data[k] = '\0';
			}
		}
	}
}

void writeImage(int aux, int imagefd, int socketfd) {
    unsigned char type;
    char source[15];
	char character;
	char data[8];

	while (aux > 0) {
		readTimes(socketfd,15,source);
		read(socketfd, &character, sizeof(char));
		
		for (int i = 0; i < 29; i++) {
        	readTimes(socketfd, 8, data);
        	type = strtol(data, 0, 2);
			if (aux > 0) {
				write(imagefd, &type, 1);
        	}
			aux--;
		}
		readTimes(socketfd, 8, data);
    }
}
