
CC = gcc
CFLAGS = -Wall -O -fno-omit-frame-pointer -ggdb -gdwarf-2
# 添加c文件目录
OBJ = udp_client udp_server udp_connect_client udp_connect_server

all:$(OBJ)

%:%.c
	$(CC) $(CFLAGS) -o $@ $^

.PHONY : clean
clean:
	rm $(OBJ)
