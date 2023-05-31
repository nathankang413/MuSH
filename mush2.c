#include "mush2.h"

int main(int argc, char *argv[]) {
    int o, v=0, p=0;
    char *cmd_file=NULL;

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

    /* get command file, if exists */
    if (argc - optind == 1) {
        cmd_file = argv[optind++];
    }

    /* verbose: print options */
    if (v) {
        printf("Options:\n");
        printf("  int   opt_verbose   = %d\n", v);
        printf("  int   opt_parseonly = %d\n", p);
        printf("  char *opt_mode      = %s\n", cmd_file ? cmd_file : "(null)");
    }

    return EXIT_SUCCESS;
}
