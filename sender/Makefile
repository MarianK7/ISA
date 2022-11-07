CC=gcc
CFLAGS= -std=gnu99 -Wall -Wextra -Werror -pedantic -pthread
BIN=sender
SOURCE=dns_sender.c
PARAMS=-u 127.0.0.1 example.com data.txt 

all:
	$(CC) $(CFLAGS) $(SOURCE) -o $(BIN)

clean:
	rm $(BIN) *.out	

run:
	./$(BIN) $(PARAMS)

out:
	cat proj2.out