#ifndef _UTILITIES_H_
#define _UTILITIES_H_

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <time.h>
#include <pthread.h>

typedef struct {
	char* ip;
	int port;
	char* folder;
} ConfigurationAtreides;

typedef struct {
    int cleanUp;
    char* ip;
    int port;
    char* folder;
} Configuration;

typedef struct {
	char* id;
	char* name;
} Search;

int readConfigFile(Configuration* config, char *argv);

int readConfigFileAtreides(ConfigurationAtreides * config, char *argv);

int readMainMemoryName(char* name, char* postalCode, char* path); 

Search* readMainMemorySearch(int* aux, int postalCode, char* path);

void convertLowerCase(char* string);

char** parseString(char* string);

int convertToInt(char* string);

char* convertToPoint(int number);

void initArray(char* buffer, int number);

#endif
