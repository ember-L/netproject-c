#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "lib/tcp_server.h"

struct Buffer;
struct Buffer *alloc_Buffer();
void free_Buffer(struct Buffer *buffer);
int onSocketRead(int fd, struct Buffer *buffer);
int onSocketWrite(int fd, struct Buffer *buffer);
int socketSetNonBlockNoDelay(int fd);
int createNonblockTCPServer(int port);
int acceptClient(int listenfd);

#endif