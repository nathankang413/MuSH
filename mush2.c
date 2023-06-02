#include "mush2.h"

int main(int argc, char *argv[]) {
    int o, v=0, p=0;
    char *cmd_path=NULL, *line;
    FILE *cmd_file=stdin;
    struct pipeline *pl;

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

    if (!p) { /* todo: implement parseonly */
        fprintf(stderr, "mush2: only parseonly mode has been implemented\n");
        return EXIT_FAILURE;
    }

    /* CLI loop */
    while (!feof(cmd_file)) {
        if (!(line = readLongString(cmd_file))) {
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

        free(line);
        free_pipeline(pl);
    }

    return EXIT_SUCCESS;
}
