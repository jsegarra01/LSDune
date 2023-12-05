#include "cmd.h"


pid_t spawnChild(const char* program, char** arg_list){
    pid_t ch_pid = fork();
    if (ch_pid == -1) {
       	free(arg_list);
	   	perror("fork");
        exit(EXIT_FAILURE);
    }
    if (ch_pid > 0) {
        free(arg_list);
		return ch_pid;
    } else {
        execvp(program, arg_list);
        perror("execve");
		free(arg_list);
        exit(EXIT_FAILURE);
    }
}

void pipeFork(char** program, char* buffer){	
	pid_t ch_pid;
	int link[2];

 	if (pipe(link)==-1){
        exit(EXIT_FAILURE);
    }


	ch_pid = fork();
	if (ch_pid == -1) { 
		perror("fork"); 
		exit(EXIT_FAILURE);
	}
	else {
  		if (ch_pid == 0) {											//Child gets executes the function and obtains the mdSUM
			dup2 (link[1], STDOUT_FILENO);
    		close(link[0]);
    		//open(link[1]);
			execvp("md5sum", program);
			exit(EXIT_FAILURE);
		}
  		else {
			wait(NULL);
			close(link[1]);
    		int bytes = read(link[0], buffer, sizeof(char)*240);
    		close(link[0]);
			buffer[bytes - 1] = '\0'; 
			buffer = strtok(buffer," ");
		}
	}
}



void getMd5sum(char *path, char* buffer) {
	char* program[3];
     
	program[0] = "md5sum";
	program[1] = path;
	program[2] = NULL;

    pipeFork(program, buffer);
}

