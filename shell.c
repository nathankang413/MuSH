#include "shell.h"

void shell_loop(FILE *cmd_file, int v, int p) {
    int i;
    char *line;
    struct sigaction sa;
    sigset_t sigint_mask;
    struct pipeline *pl;
    struct clstage curr;
    char *home;
    pid_t child;
    int in_fd, out_fd, next_in_fd, pipefd[2];
    int status;

    /* prep procmask */
    if (sigemptyset(&sigint_mask) == -1) {
        perror("sigemptyset");
        exit(EXIT_FAILURE);
    }
    if (sigaddset(&sigint_mask, SIGINT) == -1) {
        perror("sigaddset");
        exit(EXIT_FAILURE);
    }

    /* prep sigaction */
    sa.sa_handler = sigint_handler;
    sa.sa_flags = 0;
    if (sigemptyset(&sa.sa_mask) == -1) {
        perror("sigemptyset");
        exit(EXIT_FAILURE);
    }

    /* set sigaction */
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (v >= 2) {
        printf("SIGINT handler installed.\n");
        fflush(stdout);
    }

    /* CLI loop */
    while (!feof(cmd_file)) {   /* todo: all error continues skip freeing memory */

        /* print prompt */
        if (isatty(fileno(cmd_file)) && isatty(fileno(stdout))) {
            printf("8-P ");
            fflush(stdout);
        }

        /* read a command */
        if (!(line = readLongString(cmd_file))) {
            if (clerror && clerror != E_EMPTY) {
                fprintf(stderr, "readLongString: clerror %d\n", clerror);
            }
            continue;
        }

        /* split the command into a pipeline */
        if (!(pl = crack_pipeline(line))) {
            /* fprintf(stderr, "crack_pipeline: clerror %d\n", clerror); */
            free(line);
            continue;
        }

        if (v) {
            print_pipeline(stdout, pl);
        }

        if (p) {
            free(line);
            free_pipeline(pl);
            continue;
        }

        /* loop through all the stages in the pipeline */
        next_in_fd = -1;
        for (i=0; i<pl->length; i++) {
            curr = pl->stage[i];

            /* handle cd command */
            if (strcmp(curr.argv[0], "cd") == 0) {
                if (curr.argc > 1) {
                    if (chdir(curr.argv[1]) == -1) {
                        perror(curr.argv[1]);
                    }
                } else {
                    if ((home = gethome())) {
                        if (chdir(home) == -1) {
                            perror(home);
                        }
                    } else {
                        fprintf(stderr, 
                        "cd: unable to determine home directory\n");
                    }
                }
                break;
            }

            /* handle exit command */
            if (strcmp(curr.argv[0], "exit") == 0) {
                free(line);
                free_pipeline(pl);
                return ;
            }

            /* if there is an input file, open it*/
            if (curr.inname) {
                if ((in_fd = open(curr.inname, O_RDONLY)) == -1) {
                    perror(curr.inname);
                    break;
                }
            }
            /* if there is a pipe from the previous program, use that */
            else if (next_in_fd != -1) {
                in_fd = next_in_fd;
            } else {
                in_fd = -1;
            }

            /* if there is an output file, open it */
            if (curr.outname) {
                if ((out_fd = open(curr.outname, 
                    O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1) {
                    perror(curr.outname);
                    break;
                }
                next_in_fd = -1;
            }

            /* if there is a next program, make a pipe */
            else if (i < pl->length - 1) {
                if (pipe(pipefd) == -1) {
                    perror("pipe");
                    break;
                }
                out_fd = pipefd[1];
                next_in_fd = pipefd[0];
            } else {
                out_fd = -1;
                next_in_fd = -1;
            }

            /* create the child process  */
            if ((child = fork()) == -1) {
                perror("fork");
                break;
            }

            /* todo: turn off signal handler during this time? */

            if (child == 0) {
                /* set sigaction to default for child */
                sa.sa_handler = SIG_DFL;
                if (sigaction(SIGINT, &sa, NULL) == -1) {
                    perror("sigaction");
                    exit(EXIT_FAILURE);
                }
                if (v >= 2) {
                    printf("SIGINT handler uninstalled (pid=%d).\n", getpid());
                }
                run_child(curr.argv, in_fd, out_fd, next_in_fd);
                exit(EXIT_FAILURE);  /* safety precaution */
            }

            /* parent process will ignore sigint until child finishes */
            if (sigprocmask(SIG_BLOCK, &sigint_mask, NULL) == -1) {
                perror("sigprocmask");
                break;
            }
            if (v >= 2) {
                printf("SIGINT blocked (pid=%d).\n", getpid());
            }

            /* parent process closes fds except next_in_fd */
            if (in_fd != -1) 
                close(in_fd);
            if (out_fd != -1) 
                close(out_fd);

            /* parent process waits for child */
            while (waitpid(child, &status, 0) == -1) {
                if (errno != EINTR) {
                    perror("waitpid");
                    break;
                }
            }

            /* accept sigint again */
            if (sigprocmask(SIG_UNBLOCK, &sigint_mask, NULL) == -1) {
                perror("sigprocmask");
                break;
            }
            if (v >= 2) {
                printf("SIGINT unblocked (pid=%d).\n", getpid());
            }
        }

        free(line);
        free_pipeline(pl);
    }
}

/* only used when shell in "ready" state */
void sigint_handler(int signal) {
    printf("\n");
    fflush(stdout);
}

/* get the home directory of the user */
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