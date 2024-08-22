
CC = gcc
CFLAGS = -Wall -O -fno-omit-frame-pointer -ggdb -gdwarf-2
OBJ = tcp_client tcp_server

all:tcp_client.c tcp_server.c
	$(CC) $(CFLAGS) tcp_client.c -o tcp_client 
	$(CC) $(CFLAGS) tcp_server.c -o tcp_server

.PHONY : clean
clean:
	rm $(OBJ)
