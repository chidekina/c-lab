#ifndef MYSH_H
#define MYSH_H

#define MYSH_VERSION "0.1.0"
#define MAX_INPUT    1024
#define MAX_ARGS     64

/* Read a line from stdin into buf (max MAX_INPUT bytes).
 * Returns number of bytes read, or -1 on EOF. */
int mysh_readline(char *buf, int size);

/* Split buf into tokens (space-separated).
 * Writes pointers into argv, returns argc.
 * argv must have room for MAX_ARGS pointers. */
int mysh_tokenize(char *buf, char **argv);

/* Execute a builtin command. Returns 1 if handled, 0 if not a builtin. */
int mysh_builtin(int argc, char **argv);

#endif /* MYSH_H */
