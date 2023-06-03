#include "mush2.h"

int main(int argc, char *argv[]) {
    int o, v=0, p=0;
    char *cmd_path=NULL, *line;
    FILE *cmd_file=stdin;
    struct sigaction sa;
    struct pipeline *pl;
    struct clstage *curr;
    pid_t child;
    int in_fd, out_fd, next_in_fd, pipefd[2];
    int status;

    /* parse options */
    while ((o = getopt(argc, argv, "vp")) != -1) {
        switch (o) {
            case 'v': v++; break;
            case 'p': p++; break;
            default:
                fprintf(stderr, "%s: invalid option -- '%c'\n", argv[0], o);
                fprintf(stderr, USAGE);
                return EXIT_FAILURE;
        }
    }

    /* too many args */
    if (argc - optind > 1) {
        fprintf(stderr, USAGE);
        return EXIT_FAILURE;
    }

    /* setup command file, if exists */
    if (argc - optind == 1) {
        cmd_path = argv[optind++];
    }
    if (cmd_path) {
        if (!(cmd_file = fopen(cmd_path, "r"))) {
            perror(cmd_path);
            return EXIT_FAILURE;
        }
    }

    /* verbose: print options */
    if (v) {
        printf("Options:\n");
        printf("  int   opt_verbose   = %d\n", v);
        printf("  int   opt_parseonly = %d\n", p);
        printf("  char *opt_mode      = %s\n", cmd_path ? cmd_path : "(null)");
        fflush(stdout);
    }

    /* todo: handle sigint */
    sa.sa_handler = sigint_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    /* CLI loop */
    while (!feof(cmd_file)) {

        if (isatty(fileno(cmd_file))) {
            printf("8-P ");
            fflush(stdout);
        }

        if (!(line = readLongString(cmd_file))) {
            if (clerror)
                fprintf(stderr, "readLongString: clerror %d\n", clerror);
            continue;
        }

        if (!(pl = crack_pipeline(line))) {
            /* fprintf(stderr, "crack_pipeline: clerror %d\n", clerror); */
            continue;
        }

        if (v) {
            print_pipeline(stdout, pl);
        }

        if (!p && pl->length > 0) { /* actually run the command */

            /* need to implement cd */


            /* loop through all the stages in the pipeline */
            curr = pl->stage;
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
                    /* child process sets up IO */
                    if (in_fd != -1) {
                        if (dup2(in_fd, STDIN_FILENO) == -1) {
                            perror("dup2");
                            break;
                        }
                        close(in_fd);
                    }
                    if (out_fd != -1) {
                        if (dup2(out_fd, STDOUT_FILENO) == -1) {
                            perror("dup2");
                            break;
                        }
                        close(out_fd);
                    }
                    close(next_in_fd);

                    /* child process execs */
                    execvp(pl->stage->argv[0], pl->stage->argv);
                    perror(pl->stage->argv[0]);
                    return EXIT_FAILURE;
                }

                /* parent process closes fds except next_in_fd */
                close(in_fd);
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
    }

    yylex_destroy();
    return EXIT_SUCCESS;
}


void sigint_handler(int signal) {
    putchar('\n');
    fflush(stdout);
    /* does nothing (todo) */
    /* very bad things happen when I nest mush2 (todo) */
}
