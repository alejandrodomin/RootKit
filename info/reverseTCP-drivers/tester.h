#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>
#include <sys/ioctl.h>
#include "reverseTCP_ioctl.h"

#define BUFFER_SIZE 256 
#define WAITTIME 100000
#define MAXWAIT 200
static char buffer[BUFFER_SIZE];  