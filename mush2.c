#include "mush2.h"

/* todo: memory issues */

int main(int argc, char *argv[]) {
    int o, v=0, p=0;
    char *cmd_path=NULL;
    FILE *cmd_file=stdin;
    
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

    shell_loop(cmd_file, v, p);

    yylex_destroy();
    if (v) 
        printf("VERB: Successful exit!\n"); 

    return EXIT_SUCCESS;
}
