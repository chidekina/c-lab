#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mysh.h"

int mysh_builtin(int argc, char **argv) {
    if (argc == 0)
        return 0;

    /* exit */
    if (strcmp(argv[0], "exit") == 0) {
        int code = (argc > 1) ? atoi(argv[1]) : 0;
        exit(code);
    }

    /* echo */
    if (strcmp(argv[0], "echo") == 0) {
        for (int i = 1; i < argc; i++) {
            printf("%s", argv[i]);
            if (i < argc - 1) printf(" ");
        }
        printf("\n");
        return 1;
    }

    /* pwd */
    if (strcmp(argv[0], "pwd") == 0) {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL)
            printf("%s\n", cwd);
        return 1;
    }

    return 0;
}
