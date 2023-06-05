#ifndef SHELLH
#define SHELLH

#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <pwd.h>

#include <mush.h>
#include "child.h"

void shell_loop(FILE *cmd_file, int v, int p);

void sigint_handler(int signal);

char *gethome();

#endif