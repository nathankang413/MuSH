#include "child.h"

void run_child (char **argv, int in_fd, int out_fd, int next_in_fd) {
    sigset_t sigint_mask;

    /* todo: unblock signint for child */
    if (sigemptyset(&sigint_mask) == -1) {
        perror("sigemptyset");
        exit(EXIT_FAILURE);
    }
    if (sigaddset(&sigint_mask, SIGINT) == -1) {
        perror("sigaddset");
        exit(EXIT_FAILURE);
    }
    if (sigprocmask(SIG_UNBLOCK, &sigint_mask, NULL) == -1) {
        perror("sigprocmask");
        exit(EXIT_FAILURE);
    }
    /* 
    sa.sa_handler = SIG_DFL;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        break;
    }
    */

    /* child process sets up IO */
    if (in_fd != -1) {
        if (dup2(in_fd, STDIN_FILENO) == -1) {
            perror("dup2");
            exit(EXIT_FAILURE);
        }
        close(in_fd);
    }
    if (out_fd != -1) {
        if (dup2(out_fd, STDOUT_FILENO) == -1) {
            perror("dup2");
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