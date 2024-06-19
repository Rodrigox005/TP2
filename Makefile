CC = gcc
CFLAGS = -Wall -pthread

all: client server

client: client.o common.o
	$(CC) $(CFLAGS) client.o common.o -o client

server: server.o common.o
	$(CC) $(CFLAGS) server.o common.o -o server

client.o: client.c common.h
	$(CC) $(CFLAGS) -c client.c

server.o: server.c common.h
	$(CC) $(CFLAGS) -c server.c

common.o: common.c common.h
	$(CC) $(CFLAGS) -c common.c

clean:
	rm -f *.o client server
