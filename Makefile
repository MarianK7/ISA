CC=gcc
CFLAGS= -std=gnu99 -Wall -Wextra -pedantic -pthread
BIN=sender
BIN2=reciever
SOURCE=dns_sender.c
SOURCE2=dns_reciever.c
PARAMS=-u 127.0.0.1 www.example.com data.txt ./banana.jpg
PARAMS2=www.example.com ./data

all:
	cd sender && $(CC) $(CFLAGS) -o $(BIN) $(SOURCE)
	cd receiver && $(CC) $(CFLAGS) -o $(BIN2) $(SOURCE2)

clean:
	cd sender && rm $(BIN)	
	cd receiver && rm $(BIN2)

run:
	cd sender && sudo ./$(BIN) $(PARAMS)

run2:
	cd receiver && sudo ./$(BIN2) $(PARAMS2)
