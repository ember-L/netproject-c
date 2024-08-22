#include "../include/buffer.h"
#include <bits/types/struct_iovec.h>
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>
#include <sys/mman.h>

const char* CRLF = "\r\n";

void make_room(struct buffer *buffer, int size) {
    if (buffer_writeable_size(buffer) >= size) {
        return;
    }
    //如果front_spare和writeable的大小加起来可以容纳数据，则把可读数据往前面拷贝
    if (buffer_front_spare_size(buffer) + buffer_writeable_size(buffer) >= size) {
        int readable = buffer_readable_size(buffer);
        for (int i = 0; i < readable; i++) {
            memcpy(buffer->data + i, buffer->data + buffer->readIndex + i, 1);
        }
        buffer->readIndex = 0;
        buffer->writeIndex = readable;
    } else {
        //扩大缓冲区
        void *tmp = realloc(buffer->data, buffer->total_size + size);
        if (tmp == NULL) {
            return;
        }
        buffer->data = tmp;
        buffer->total_size += size;
    }
}

struct buffer *buffer_new(){
    struct buffer *buf = malloc(sizeof(struct buffer));

    if(!buf)
        return NULL;

    buf->data = malloc(sizeof(char) * INIT_BUFFER_SIZE);
    memset(buf->data, 0, sizeof(char) * INIT_BUFFER_SIZE);

    buf->total_size = INIT_BUFFER_SIZE;
    buf->readIndex = buf->writeIndex = 0;

    /*for mmap*/
    buf->file_base = NULL;
    buf->file_offset = buf->file_size = 0;

    return buf;
}

void buffer_free(struct buffer* buffer){
    free(buffer->data);
    free(buffer);
}

int buffer_writeable_size(struct buffer* buffer) {
    return buffer->total_size - buffer->writeIndex;
}

int buffer_readable_size(struct buffer* buffer) {
    return buffer->writeIndex - buffer->readIndex;
}

int buffer_front_spare_size(struct buffer *buffer) {
    return buffer->readIndex;
}

int buffer_append(struct buffer *buffer, void *data, int size) {
    if (data) {
        //扩充空间
        make_room(buffer, size);
        memcpy(buffer->data + buffer->writeIndex, data, size);
        buffer->writeIndex += size;
    }
    return 0;
}

int buffer_append_char(struct buffer *buffer, char data) {
    if (data) {
        buffer->data[buffer->writeIndex++] = data;
    }
    return 0;
}

int buffer_append_string(struct buffer *buffer, char *data) {
    if (data) {
        int size = strlen(data);
        buffer_append(buffer, data, size);
    }
    return 0;
}

int buffer_socket_read(struct buffer *buffer, int fd) {
    char additional_buffer[INIT_BUFFER_SIZE];
    struct iovec vec[2];

    int max_writable = buffer_writeable_size(buffer);

    vec[0].iov_base = buffer->data + buffer->writeIndex;
    vec[0].iov_len = max_writable;

    vec[1].iov_base = additional_buffer;
    vec[1].iov_len = sizeof(additional_buffer);
    
    int bytes_have_received = readv(fd, vec, 2);

    if(bytes_have_received < 0)
        return -1;
    else if (bytes_have_received <= max_writable) {
        buffer->writeIndex += bytes_have_received;
    } else {
        buffer->writeIndex = buffer->total_size;
        buffer_append(buffer, additional_buffer, bytes_have_received - max_writable);
    }

    return bytes_have_received;
}

char buffer_read_char(struct buffer *buffer) {
    return buffer->data[buffer->readIndex++];
}

char *buffer_find_CRLF(struct buffer *buffer) {
    char *crlf = strstr(buffer->data + buffer->readIndex , CRLF);
    return crlf;
}

/*文件缓冲*/
int buffer_file_sendable_size(struct buffer *buffer) {
    return buffer->file_size - buffer->file_offset;
}

void buffer_file_free(struct buffer *buffer){
    if(buffer->file_base){
        munmap(buffer->file_base, buffer->file_size);
        buffer->file_base = NULL;
        buffer->file_size = buffer->file_offset = 0;
    }
}