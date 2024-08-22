#include "../include/tcp_connection.h"
#include "../include/log.h"
#include <errno.h>
#include <error.h>
#include <pthread.h>
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

int handle_connection_closed(struct tcp_connection *tcpConnection) {
    struct event_loop *eventLoop = tcpConnection->eventLoop;
    struct channel *chann = tcpConnection->channel;

    event_loop_remove_channel_event(eventLoop, chann);

    if (tcpConnection->connectionClosedCallBack) 
        tcpConnection->connectionClosedCallBack(tcpConnection);

    tcp_connection_free(tcpConnection);
    return 0;
}

int handle_read(void *data) {
    struct tcp_connection *tcpConnection = (struct tcp_connection *)data;
    struct channel *chann = tcpConnection->channel;
    ember_debugx("read socket from fd == %d, %s", chann->fd, tcpConnection->eventLoop->thread_name);
    if (buffer_socket_read(tcpConnection->input_buffer, chann->fd) > 0) {
        /*更新定时器*/
        update_timer(tcpConnection->timer, TIMEOUT);
        reset_timer(tcpConnection->timer->list , tcpConnection->timer);
        
        if (tcpConnection->messageCallBack)
            tcpConnection->messageCallBack(tcpConnection);
    }else {
        del_timer(tcpConnection->timer->list, tcpConnection->timer);
        handle_connection_closed(tcpConnection);
    }
    return 0;
}

int handle_write(void *data) {
    struct tcp_connection *tcpConnection = (struct tcp_connection *)data;
    struct event_loop *eventLoop = tcpConnection->eventLoop;
    assert(eventLoop->owner_thread_id == pthread_self());

    struct buffer *output_buffer= tcpConnection->output_buffer;
    struct channel *chann =  tcpConnection->channel;

    size_t bytes_have_sent = 0;
    int temp = write(chann->fd, output_buffer->data + output_buffer->readIndex,
                        buffer_readable_size(output_buffer));

    if (temp >= 0) {
        output_buffer->readIndex += temp;

        bytes_have_sent += temp;
        // 如果数据完全发送，就停止
        if (buffer_readable_size(output_buffer) == 0){
            chann->data = tcpConnection->eventLoop;
            channel_write_event_disable(chann);
            chann->data= tcpConnection;
        }
        
        if (tcpConnection->writeCompletedCallBack)
            tcpConnection->writeCompletedCallBack(tcpConnection);
    } else {
        if (errno == EAGAIN){
            ember_msgx("TCP buffer is filled %s", tcpConnection->name);
            channel_write_event_enable(chann);
            return bytes_have_sent;
        }
        if (errno != EWOULDBLOCK) {
            if (errno == EPIPE || errno == ECONNRESET) {
                error(-1, errno, "write buffer occur error");       
            }
        }
        return -1;
    }
    
    return bytes_have_sent;
}

struct tcp_connection * 
tcp_connection_new(int connected_fd, struct event_loop *eventLoop,
                    connection_completed_call_back connectionCompletedCallBack,
                    connection_closed_call_back connectionClosedCallBack,
                    message_call_back messageCallBack, 
                    write_completed_call_back writeCompletedCallBack) {
    struct tcp_connection *tcpConnection = malloc(sizeof(struct tcp_connection));

    tcpConnection->connectionCompletedCallBack =  connectionCompletedCallBack;
    tcpConnection->connectionClosedCallBack = connectionClosedCallBack;
    tcpConnection->messageCallBack = messageCallBack;
    tcpConnection->writeCompletedCallBack = writeCompletedCallBack;

    tcpConnection->eventLoop = eventLoop;
    tcpConnection->input_buffer = buffer_new();
    tcpConnection->output_buffer = buffer_new();

    char *buf = malloc(16);
    sprintf(buf, "connect[%d]", connected_fd);   
    tcpConnection->name = buf;

    struct channel *chann = channel_new(connected_fd, EVENT_READ, handle_read, handle_write, tcpConnection);
    tcpConnection->channel = chann;

    if (tcpConnection->connectionCompletedCallBack) 
        tcpConnection->connectionCompletedCallBack(tcpConnection);

    event_loop_add_channel_event(tcpConnection->eventLoop, chann);
    
    return tcpConnection;
}

void tcp_connection_free(struct tcp_connection *tcpConnection){
    buffer_free(tcpConnection->input_buffer);
    buffer_free(tcpConnection->output_buffer);
    free(tcpConnection);
}

// 应用层服务
int tcp_connection_open_write_channel(struct tcp_connection *tcpConnection){
    struct buffer *output = tcpConnection->output_buffer; 
    struct channel *chann = tcpConnection->channel;

    if(!channel_write_event_is_enabled(chann) && buffer_readable_size(output) > 0){
        chann->data = tcpConnection->eventLoop;
        channel_write_event_enable(chann);
        chann->data = tcpConnection;
    }
    return 0;
}

int tcp_connection_send_data(struct tcp_connection *tcpConnection, void *data, int size) {
    size_t nleft = size , n = 0;
    int fault = 0;

    struct channel *channel = tcpConnection->channel;
    struct buffer *output_buffer = tcpConnection->output_buffer;
    //尝试往缓冲区写入数据
    if (!channel_write_event_is_enabled(channel) && buffer_readable_size(output_buffer) == 0) {        
        
        n = write(channel->fd, data, size);
        ember_debugx("send msg to fd == %d size = %d n = %d", channel->fd, size , n);
        if (n >= 0) {
            nleft -= n;
        } else {
            if (errno == EAGAIN){
                ember_debugx("TCP buffer is filled");
            }
            
            n = 0;
            if (errno != EWOULDBLOCK) {
                if (errno == EPIPE || errno == ECONNRESET) {
                    fault = 1;
                }
            }
        }
    }
    if (!fault && nleft > 0) {
        // 拷贝到buffer，由buffer管理内容
        buffer_append(output_buffer, data + n, nleft);
        if (!channel_write_event_is_enabled(channel)) {
            channel_write_event_enable(channel);
        }
    }
    return n;
}

int tcp_connection_send_buffer(struct tcp_connection *tcpConnection, struct buffer *buffer){
    int size = buffer_readable_size(buffer);
    int result = tcp_connection_send_data(tcpConnection, buffer->data,  size);
    buffer->readIndex += size;
    return result;
}

int tcp_connection_shutdown(struct tcp_connection *tcpConnection){
    if (shutdown(tcpConnection->channel->fd, SHUT_WR) < 0) {
        ember_msgx("tcp_connection shutdown failed fd = %d", tcpConnection->channel->fd);
    }
    return 0;
}