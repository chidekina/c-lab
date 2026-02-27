CC      = gcc
CFLAGS  = -Wall -Wextra -Werror -g -std=c17 -Iinclude
SAN     = -fsanitize=address,undefined
SRCDIR  = src
BINDIR  = bin
BIN     = $(BINDIR)/mysh

SRCS    = $(wildcard $(SRCDIR)/*.c)
OBJS    = $(SRCS:.c=.o)

all: $(BINDIR) $(BIN)

$(BINDIR):
	mkdir -p $(BINDIR)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

run: all
	./$(BIN)

valgrind: all
	valgrind --leak-check=full --track-origins=yes ./$(BIN)

san: $(BINDIR)
	$(CC) $(CFLAGS) $(SAN) -o $(BINDIR)/mysh-san $(SRCS)
	./$(BINDIR)/mysh-san

clean:
	rm -f $(SRCDIR)/*.o $(BIN) $(BINDIR)/mysh-san

.PHONY: all run valgrind san clean
