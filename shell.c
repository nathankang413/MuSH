#include "shell.h"

void shell_loop(FILE *cmd_file, int v, int p) {
    char *line;
    /* struct sigaction sa; */
    sigset_t sigint_mask;
    struct pipeline *pl;
    struct clstage *curr;
    char *home;
    pid_t child;
    int in_fd, out_fd, next_in_fd, pipefd[2];
    int status;

    /* todo: handle sigint */
    if (sigemptyset(&sigint_mask) == -1) {
        perror("sigemptyset");
        exit(EXIT_FAILURE);
    }
    if (sigaddset(&sigint_mask, SIGINT) == -1) {
        perror("sigaddset");
        exit(EXIT_FAILURE);
    }
    if (sigprocmask(SIG_BLOCK, &sigint_mask, NULL) == -1) {
        perror("sigprocmask");
        exit(EXIT_FAILURE);
    }
    /* 
    sa.sa_handler = sigint_handler;
    sa.sa_flags = 0;
    if (sigemptyset(&sa.sa_mask) == -1) {
        perror("sigemptyset");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    */

    /* CLI loop */
    while (!feof(cmd_file)) {   /* todo: all error continues skip freeing memory */

        /* print prompt */
        if (isatty(fileno(cmd_file))) {
            printf("8-P ");
            fflush(stdout);
        }

        /* read a command */
        if (!(line = readLongString(cmd_file))) {
            if (clerror)
                fprintf(stderr, "readLongString: clerror %d\n", clerror);
            continue;
        }

        /* split the command into a pipeline */
        if (!(pl = crack_pipeline(line))) {
            /* fprintf(stderr, "crack_pipeline: clerror %d\n", clerror); */
            continue;
        }

        if (v) {
            print_pipeline(stdout, pl);
        }

        /* actually run the command */
        if (!p && pl->length > 0) {

            curr = pl->stage;

            /* handle cd */
            if (strcmp(curr->argv[0], "cd") == 0) {
                if (curr->argc > 1) {
                    chdir(curr->argv[1]);
                } else {
                    if ((home = gethome())) {
                        chdir(home);
                    } else {
                        fprintf(stderr, 
                        "cd: unable to determine home directory\n");
                    }
                }
                continue;
            }

            /* handle exit */
            if (strcmp(curr->argv[0], "exit") == 0) {
                break;
            }

            /* loop through all the stages in the pipeline */
            in_fd = -1; out_fd = -1; next_in_fd = -1;
            while (curr) {

                /* if there is an input file, open it*/
                if (curr->inname) {
                    if ((in_fd = open(curr->inname, O_RDONLY)) == -1) {
                        perror(curr->inname);
                        break;
                    }
                }
                /* if there is a pipe from the previous program, use that */
                else if (next_in_fd != -1) {
                    in_fd = next_in_fd;
                } 

                /* if there is an output file, open it */
                if (curr->outname) {
                    if ((out_fd = open(curr->outname, 
                        O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1) {
                        perror(curr->outname);
                        break;
                    }
                    next_in_fd = -1;
                }
                /* if there is a next program, make a pipe */
                else if (curr->next) {
                    if (pipe(pipefd) == -1) {
                        perror("pipe");
                        break;
                    }
                    out_fd = pipefd[1];
                    next_in_fd = pipefd[0];
                }

                /* create the child process  */
                if ((child = fork()) == -1) {
                    perror("fork");
                    break;
                }

                if (child == 0) {
                    run_child(curr->argv, in_fd, out_fd, next_in_fd);
                }

                /* parent process closes fds except next_in_fd */
                if (in_fd != -1) 
                    close(in_fd);
                if (out_fd != -1) 
                    close(out_fd);

                /* parent process waits for child */
                if (waitpid(child, &status, 0) == -1) {
                    perror("waitpid");
                    break;
                }
                
                curr = curr->next;
            }
        }

        free(line);
        free_pipeline(pl);

        printf("\n");
        fflush(stdout);
    }
}

void sigint_handler(int signal) {
    printf("\n");
    fflush(stdout);
    /* does nothing (todo) */
    /* very bad things happen when I nest mush2 (todo) */
}

char *gethome() {
    char *home;
    struct passwd *pw;

    if ((home = getenv("HOME"))) {
        return home;
    }

    if ((pw = getpwuid(getuid()))) {
        return pw->pw_dir;
    }

    return NULL;
}