#ifndef __TCP_CONNECT_H
#define __TCP_CONNECT_H

#include "event_loop.h"
#include "channel.h"
#include "timer.h"
#include "buffer.h"
#include "tcp_utils.h"

struct tcp_connection {
    struct event_loop *eventLoop;
    struct channel *channel;
    char *name;
    struct buffer *input_buffer;
    struct buffer *output_buffer;

    connection_completed_call_back connectionCompletedCallBack;
    connection_closed_call_back connectionClosedCallBack;
    message_call_back messageCallBack;
    write_completed_call_back writeCompletedCallBack;

    /*定时器*/
    struct timer *timer;

    void *data; //for http
    void *request;
    void *response;
};

struct tcp_connection *
tcp_connection_new(int,struct event_loop*, connection_completed_call_back,
        connection_closed_call_back, message_call_back, write_completed_call_back);
//应用层调用接口
int handle_write(void *);

int tcp_connection_send_data(struct tcp_connection *, void*, int);

int tcp_connection_send_buffer(struct tcp_connection *, struct buffer *);

int tcp_connection_open_write_channel(struct tcp_connection *);

int handle_connection_closed(struct tcp_connection *);

int tcp_connection_shutdown(struct tcp_connection *);

void tcp_connection_free(struct tcp_connection*);

#endif