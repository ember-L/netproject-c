
CC = gcc
CFLAGS = -Wall -O -fno-omit-frame-pointer -ggdb -gdwarf-2
# 添加c文件目录
OBJ = select_nonblock

all:$(OBJ)

%:%.c
	$(CC) $(CFLAGS) -o $@ $^

.PHONY : clean
clean:
	rm $(OBJ)
