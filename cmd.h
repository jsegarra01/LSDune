#ifndef _CMD_H_
#define _CMD_H_

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include "utilities.h"

pid_t spawnChild(const char* program, char** arg_list);

void getMd5sum(char* path, char* buffer);

#endif
