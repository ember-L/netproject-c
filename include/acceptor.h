#ifndef __ACCEPTOR_H
#define __ACCEPTOR_H

struct acceptor {
    int listen_fd;
    int listen_port;
};

struct acceptor* acceptor_init(int);

#endif