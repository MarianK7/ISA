CC=gcc
CFLAGS= -std=gnu99 -Wall -Wextra -Werror -pedantic -pthread
BIN=sender
BIN2=reciever
SOURCE=dns_sender.c
SOURCE2=dns_reciever.c
PARAMS=-u 127.0.0.1 example.com data.txt 
PARAMS2=name destination.txt

all:
	cd sender && $(CC) $(CFLAGS) -o $(BIN) $(SOURCE)
	cd receiver && $(CC) $(CFLAGS) -o $(BIN2) $(SOURCE2)

clean:
	cd sender && rm $(BIN)	
	cd receiver && rm $(BIN2)

run:
	cd sender && ./$(BIN) $(PARAMS)

run2:
	cd receiver && ./$(BIN2) $(PARAMS2)
