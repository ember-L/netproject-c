#ifndef __TCP_SERVER_H
#define __TCP_SERVER_H

#include "require.h"
#include "channel.h"

typedef int (*connection_completed_call_back) (struct tcp_connection *tcpConnect);

typedef int (*message_call_back) (struct tcp_connection *tcpConnect);

typedef int (*write_completed_call_back) (struct tcp_connect *tcpConnect);

typedef int (*connection_closed_call_back) (struct tcp_connection *tcpConnect);

struct TCPServer{
    int port;
    struct event_loop *eventLoop;

    struct acceptor *acceptor;

    connection_completed_call_back connectionCompletedCallBack;
    connection_closed_call_back connectionClosedCallBack;
    message_call_back messageCallBack;
    write_completed_call_back writeCompletedCallBack;
    
    int nthread;
    struct thread_pool *threadPool;
    void *data;//for http_server   
};

struct TCPServer *
tcp_server_init(struct event_loop *,struct acceptor *,
                connection_completed_call_back ,
                connection_closed_call_back,
                write_completed_call_back,
                message_call_back, int);

void tcp_server_start(struct TCPServer *);
int socketSetNonBlockNoDelay(int fd);
int createTCPServer(int port);
int createNonblockTCPServer(int port);
int acceptClient(int listenfd);

int handle_connection_established(void *data);
void tcp_server_start_with_read(struct TCPServer *, event_read_callback);

#endif