#include "child.h"

void run_child (char **argv, int in_fd, int out_fd, int next_in_fd) {

    /* child process sets up IO */
    if (in_fd != -1) {
        if (dup2(in_fd, STDIN_FILENO) == -1) {
            perror("dup2: in_fd");
            exit(EXIT_FAILURE);
        }
        close(in_fd);
    }
    if (out_fd != -1) {
        if (dup2(out_fd, STDOUT_FILENO) == -1) {
            perror("dup2: out_fd");
            exit(EXIT_FAILURE);
        }
        close(out_fd);
    }
    if (next_in_fd != -1) {
        close(next_in_fd);
    }

    /* child process execs */
    execvp(argv[0], argv);
    perror(argv[0]);
    exit(EXIT_FAILURE); 
}