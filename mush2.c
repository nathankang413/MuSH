#include "mush2.h"

int main(int argc, char *argv[]) {
    int o, v=0, p=0;
    char *cmd_path=NULL, *line;
    FILE *cmd_file=stdin;
    struct pipeline *pl;
    pid_t child;
    int in_fd, out_fd, status;

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
            /* for just one command (todo) */
            if (pl->length > 1) {
                fprintf(stderr, "mush2: only one command has been implemented\n");
                continue;
            }

            /* open infile for reading */
            in_fd = STDIN_FILENO;
            if (pl->stage->inname) {
                if ((in_fd = open(pl->stage->inname, O_RDONLY)) == -1) {
                    perror(pl->stage->inname);
                    continue;
                }
            }

            /* open outfile for writing */
            out_fd = STDOUT_FILENO;
            if (pl->stage->outname) {
                if ((out_fd = open(pl->stage->outname, 
                    O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1) {
                    perror(pl->stage->outname);
                    continue;
                }
            }

            /* fork off child process */
            if ((child = fork()) == -1) {
                perror("fork");
                continue;
            }

            /* child will call exec */
            if (child == 0) {
                if (dup2(in_fd, STDIN_FILENO) != in_fd) {
                    close(in_fd);
                }
                if (dup2(out_fd, STDOUT_FILENO) != out_fd) {
                    close(out_fd);
                }
                execvp(pl->stage->argv[0], pl->stage->argv);
                perror(pl->stage->argv[0]);
                return EXIT_FAILURE;
            }

            /* close parent's fds to pipes or IO redirection */
            if (in_fd != STDIN_FILENO)
                close(in_fd);
            if (out_fd != STDOUT_FILENO)
                close(out_fd);

            /* parent will wait for child to die */
            if (waitpid(child, &status, 0) == -1) {
                perror("waitpid");
                return EXIT_FAILURE;    /* todo: should this be continue? */
            }

            /* todo: look at status? */

        }

        free(line);
        free_pipeline(pl);
    }

    yylex_destroy();
    return EXIT_SUCCESS;
}
