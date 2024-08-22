#include "../include/tcp_utils.h"

#define MAX_LINE 1024
#define FD_INIT_SIZE 128
#define SERVERPORT 6666

char rot13_char(char c) { 
    if(( c >= 'a' && c <= 'm') || (c >= 'A' && c <= 'M'))
        return c + 13;
    else if ((c >= 'n' && c <= 'z') || (c >= 'N' && c <= 'Z'))
        return c - 13;
    else 
        return c;
}

//数据缓冲区
struct Buffer {
    int     connect_fd;
    char    buffer[MAX_LINE];
    size_t  writeIndex;
    size_t  readIndex;
    int     readable;
};

struct Buffer *alloc_Buffer() { 
    struct Buffer *buf = malloc(sizeof(struct Buffer));
    if(!buf)
        return NULL;
    buf->connect_fd = 0;
    buf->writeIndex = buf->readIndex = buf->readable = 0;

    return buf;
}


void free_Buffer(struct Buffer *buffer){
    free(buffer);
}

int onSocketRead(int fd, struct Buffer *buffer){
    char buf[MAX_LINE];
    ssize_t result;
    while(1){
        result = recv(fd, buf, sizeof(buf), 0);
        if ( result <=- 0 )
            break;

        for(int i = 0 ; i < result ; ++i){
            if (buffer-> writeIndex < sizeof(buffer->buffer))
                buffer->buffer[buffer->writeIndex++] = rot13_char(buf[i]);
            if (buf[i] == '\n'){
                buffer->readable = 1;//设置缓冲区为可写
            }
        }
    }

    if(result == 0)
        return 1;
    else if(result < 0){
        if(errno == EAGAIN)
            return 0;
        return -1;
    }

    return 0;
}


int onSocketWrite(int fd, struct Buffer *buffer){
    while (buffer->readIndex <  buffer->writeIndex){
        ssize_t result = send(fd, buffer->buffer + buffer->readIndex, 
                              buffer->writeIndex - buffer->readIndex,0);
        if(result < 0){
            if (errno == EAGAIN)
                return 0;
            return -1;
        }
        buffer->readIndex  += result;
    }

    if(buffer->readIndex == buffer->writeIndex)
        buffer->readIndex = buffer->writeIndex = 0;

    buffer->readable = 0;

    return 0;
}

int main(int argc, char **argv) {
    int listenfd = createNonblockTCPServer(SERVERPORT);
    int maxfd;
    fprintf(stdout,"open listening fd = %d\n",listenfd);
    struct Buffer *buffers[FD_INIT_SIZE];
    for(int i = 0; i < FD_INIT_SIZE;++i){
        buffers[i] = alloc_Buffer();
    }

    fd_set readfds, writefds, exfds;

    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_ZERO(&exfds);

    while(1){
        maxfd = listenfd;
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);
        FD_ZERO(&exfds);

        FD_SET(listenfd, &readfds);

        for(int i = 0; i < FD_INIT_SIZE; ++i) {
            if(buffers[i]->connect_fd > maxfd)
                maxfd = buffers[i]->connect_fd;
            FD_SET(buffers[i]->connect_fd, &readfds);
            if(buffers[i]->readable){
                FD_SET(buffers[i]->connect_fd, &writefds);
            }
        }
        if(select(maxfd+1, &readfds, &writefds, &exfds, NULL) < 0) {
            error(1, errno, "select error");
        }

        printf("select finished\n");
        if (FD_ISSET(listenfd, &readfds)) {
            printf("listening socket readable\n");
            sleep(5);
            int fd = acceptClient(listenfd);
            if (fd < 0) {
                error(1, errno, "accept failed");
            } else if (fd > FD_INIT_SIZE) {
                error(1, 0, "too many connections");
                close(fd);
            } else {
                socketSetNonBlockNoDelay(fd);
                if(buffers[fd]->connect_fd == 0){
                    buffers[fd]->connect_fd = fd;
                } else {
                    error(1, 0, "too many connections");
                }
            }
        }

        for (int i = 0; i < maxfd + 1; ++i) {
            int r = 0;
            if (i == listenfd)
                continue;
            if (FD_ISSET(i, &readfds)) {
                r = onSocketRead(i, buffers[i]);
            }
            if (r == 0 && FD_ISSET(i, &writefds)) {
                r = onSocketWrite(i, buffers[i]);
            }
            if (r) {
                buffers[i]->connect_fd = 0;
                close(i);
            }
        }
    }
}