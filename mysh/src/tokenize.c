#include <string.h>
#include "mysh.h"

int mysh_tokenize(char *buf, char **argv) {
    int argc = 0;
    char *token = strtok(buf, " \t");
    while (token != NULL && argc < MAX_ARGS - 1) {
        argv[argc++] = token;
        token = strtok(NULL, " \t");
    }
    argv[argc] = NULL;
    return argc;
}
