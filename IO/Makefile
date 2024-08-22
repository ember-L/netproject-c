
CC = gcc
CFLAGS = -Wall -O -fno-omit-frame-pointer -ggdb -gdwarf-2
OBJ = tcp_client select

all:tcp_client.c select.c poll.c epoll.c
	$(CC) $(CFLAGS) tcp_client.c -o tcp_client 
	$(CC) $(CFLAGS) select.c -o select
	g++  $(CFLAGS) poll.c -o poll
	$(CC) $(CFLAGS) epoll.c -o epoll
.PHONY : clean
clean:
	rm $(OBJ)
