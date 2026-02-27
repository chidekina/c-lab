#include <stdio.h>
#include <string.h>
#include "mysh.h"

int mysh_readline(char *buf, int size) {
    if (fgets(buf, size, stdin) == NULL)
        return -1;
    /* Strip trailing newline */
    int len = (int)strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
        len--;
    }
    return len;
}
