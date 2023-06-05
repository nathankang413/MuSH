#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void run_child (char **argv, int in_fd, int out_fd, int next_in_fd);