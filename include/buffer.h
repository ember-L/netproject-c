#ifndef __BUFFER_H
#define __BUFFER_H

#include "require.h"


#define INIT_BUFFER_SIZE 65536
//数据缓冲区
struct buffer {
    char*   data;
    int     readIndex;
    int     writeIndex;
    int     total_size;

    /*文件映射缓冲 mmap 避免大文件拷贝*/
    char*   file_base;
    int     file_offset;
    int     file_size;
};

struct buffer* buffer_new();

void buffer_free(struct buffer*);

int buffer_writeable_size(struct buffer *);

int buffer_readable_size(struct buffer *);

int buffer_front_spare_size(struct buffer *);
//写数据
int buffer_append(struct buffer *, void *data, int size);
int buffer_append_char(struct buffer*, char data);
int buffer_append_string(struct buffer*, char *data);

//读取socker数据
int buffer_socket_read(struct buffer *, int fd);
char buffer_read_char(struct buffer* );

//查询buffer数据
char* buffer_find_CRLF(struct buffer *);

//管理文件缓冲数据
int buffer_file_sendable_size(struct buffer *);
void buffer_file_free(struct buffer *);

#endif