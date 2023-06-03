#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <mush.h>

#define USAGE "usage: %s [ -v ] [ infile ]\n", argv[0]

void sigint_handler(int signal);