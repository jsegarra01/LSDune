#include "utilities.h"

int readConfigFile(Configuration* config, char *argv) {
	char character = '0';
	int counter = 0;

	int fd_in = open(argv, O_RDONLY);
    if (fd_in == -1) {
		return 1;
	}
    else {

		config->cleanUp = 0;														//Read the cleanUp time
		do {
			config->cleanUp = config->cleanUp * 10 + character - '0';
			read(fd_in, &character,1);
 		}while (character != '\n');						
 
		config->ip = (char*) malloc(sizeof(char)); 									//Read the ip of Arrakis
		do {
			read(fd_in, &character,1);
			config->ip[counter] = character;
			counter++;
			config->ip = (char*)realloc(config->ip,sizeof(char)*(counter+1));
		}while (character != '\n');
		config->ip[counter - 1] = '\0';
		counter = 0;

		character = '0';
		config->port = 0;                                                   	 	//Read the port
        do {
			config->port = config->port * 10 + character - '0';
	        read(fd_in, &character,1);
        }while (character != '\n');
		

		config->folder = (char*) malloc(sizeof(char));                              //Read the Freeman folder
        do {
            read(fd_in, &character,1);
            config->folder[counter] = character;
            counter++;
 			config->folder = (char*)realloc(config->folder,sizeof(char)*(counter+1));
         }while (character != '\n' && character != '\0');
		config->folder[counter - 1] = '\0';

		close(fd_in);
		return 0;
	}
}

int readConfigFileAtreides(ConfigurationAtreides* config, char *argv) {
      char character = '0';
      int counter = 0;
  
      int fd_in = open(argv, O_RDONLY);
      if (fd_in == -1) {
           return 1;
      }
      else {
          config->ip = (char*) malloc(sizeof(char));                                  //Read the ip of Arrakis
          do {
              read(fd_in, &character,1);
              config->ip[counter] = character;
              counter++;
              config->ip = (char*)realloc(config->ip,sizeof(char)*(counter+1));
          }while (character != '\n');
          config->ip[counter - 1] = '\0';
          counter = 0;
 
         
		  character = '0';
          config->port = 0;                                                           //Read the port
          do {
              config->port = config->port * 10 + character - '0';
              read(fd_in, &character,1);
          }while (character != '\n');
	 	  

          config->folder = (char*) malloc(sizeof(char));                              //Read the Atreides folder
          do {
              read(fd_in, &character,1);
              config->folder[counter] = character;
              counter++;
              config->folder = (char*)realloc(config->folder,sizeof(char)*(counter+1));
          }while (character != '\n' && character != '\0');
          config->folder[counter - 1] = '\0';
 		
          close(fd_in);
          return 0;
      }
}

int readMainMemoryName(char* name, char* postalCode, char* path) {
	char buffer[200];
	char memory[200];
	char character;
	char* comparison;
	int counter = 0;
	int  notFound = 1;
	int code = 0;
	int id = 0;
	
	sprintf(buffer,"%s/mainMemory.txt", path);
	int fd = open(buffer, O_RDWR|O_CREAT|O_APPEND, 00700);
	
	if (fd == -1) {
		perror (buffer);
        return -1;
    }
    else {
		comparison = (char*) malloc(sizeof(char));
		while (read(fd, &character,1) && notFound) {
		 	comparison = (char*) realloc(comparison,sizeof(char)*2);
			
			counter = 0;
			code = 0;
			id = 0;
			do {
            	comparison[counter] = character;
            	counter++;
				comparison = (char*)realloc(comparison,sizeof(char)*(counter+1));
				read(fd, &character,1);
			} while (character != ' ');
			comparison[counter] = '\0';

			if (strcmp(comparison, name) == 0) {
				notFound = 0;
			}
			
			read(fd, &character,1);
			do {
				code = code*10 + character - '0';
				read(fd, &character,1);
			} while (character != ' ');

			read(fd, &character,1);
			do {
				id = id*10 + character - '0';
				read(fd, &character,1);
			}while (character != '\n' && character != '\0');
        }

		
		if (notFound) {
			id++;
			sprintf(memory, "%s %s %d\n", name, postalCode, id);
			write(fd, memory, strlen(memory));
		}
		
		free(comparison);
        close(fd);
        return id;
	}
}


Search* readMainMemorySearch(int* aux, int postalCode, char* path) {
	int counter, counter2 = 0, code, id;
	char buffer[200];
	char character;
	char* comparison;
	Search* search = (Search*)malloc(sizeof(Search));


    sprintf(buffer,"%s/mainMemory.txt", path);
    int fd = open(buffer, O_RDONLY);

    if (fd == -1) {
        perror (buffer);
		*aux = -1;
		return search;
    }
	else {
		comparison = (char*) malloc(sizeof(char));
		counter2 = 0;
		while (read(fd, &character,1)) {
            counter = 0;
			code = 0;
            id = 0;
            do {
                comparison[counter] = character;
                counter++;
                comparison = (char*)realloc(comparison,sizeof(char)*(counter+1));
                read(fd, &character,1);
            } while (character != ' ');
            comparison[counter] = '\0';

            read(fd, &character,1);
            do {
                code = code*10 + character - '0';
                read(fd, &character,1);
            } while (character != ' ');

            read(fd, &character,1);
            do {
                id = id*10 + character - '0';
                read(fd, &character,1);
            }while (character != '\n' && character != '\0');
			


			if (postalCode == code){
				search[counter2].name = (char*)malloc(sizeof(char));
				for(int i = 1; i <= counter; i++) {
				 	search[counter2].name = (char*)realloc(search[counter2].name,sizeof(char)*(i+1    ));
					search[counter2].name[i - 1] = comparison[i - 1];
				}
				search[counter2].name[counter] = '\0';

				//char* number = convertToPoint(id);
				//search[counter2].id = number;
				//free(number);

				search[counter2].id = convertToPoint(id);

				counter2++;
				search = (Search*)realloc(search,sizeof(Search)*(counter2 + 1));
			}


            free(comparison);
            comparison = (char*) malloc(sizeof(char));
		} 
		*aux = counter2;
		free(comparison);
		close(fd);
		return search;
	}
}



void convertLowerCase(char* string) {
    for (int i = 0;string[i] != '\0'; i++) {
        if (string[i] >= 'A' && string[i] <= 'Z') {
            string[i] = string[i] + 32;
        }
    }
}

char** parseString(char* string) {
    char** args = (char**)malloc(sizeof(char*));
    int i = 0;
    
    char *token = strtok(string, " '\n");
    args[i] = token;
    if(token != NULL){
        while(token != NULL){
            i++;
            token = strtok(NULL, " '\n");
            args = (char**)realloc(args,sizeof(char*)*(i+1));
            args[i] = token;
        }
    }
    
    args[i] = NULL;
    
	//free(string);
    return args;
}

int convertToInt(char* string) {
	int aux = 0;

	for (int i = 0; i < (int) strlen(string); i++) {
		aux = aux*10 + string[i] - '0';
	}

	return aux;
}

char* convertToPoint(int number) {
	char* string = (char*)malloc(sizeof(char)*10);
	sprintf(string, "%d", number);
	return string;
}

void initArray(char* buffer,int number){
	sprintf(buffer, "The server Atreides has closed down\n");
	while (number >= 37) {
		buffer[number--] = '\0';
	}
}
