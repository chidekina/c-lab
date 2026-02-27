CC     = gcc
CFLAGS = -Wall -Wextra -Werror -g -std=c17
SAN    = -fsanitize=address,undefined

SRC = exercises/ex01-hello.c
BIN = ex01

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

san: $(SRC)
	$(CC) $(CFLAGS) $(SAN) -o $(BIN)-san $^
	./$(BIN)-san

run: all
	./$(BIN)

clean:
	rm -f $(BIN) $(BIN)-san
