#include "../include/http_server.h"
#include "../include/acceptor.h"
#include "../include/tcp_connection.h"
#include "../include/http_request.h"
#include "../include/http_response.h"
#include "../include/log.h"
#include <bits/types/struct_iovec.h>
#include <sys/mman.h>
#include <sys/uio.h>
#include <assert.h>

int http_onConnectionCompleted(struct tcp_connection *tcpConnection) {
    printf("connect completed!!!\n");
    return 0;
}


int http_onWriteCompleted(struct tcp_connection *tcpConnect) {
    printf("write commplelet\n");
    return 0;
}

int http_onConnectionClosed(struct tcp_connection *tcpConnection) {
    printf("connection closed\n");
    return 0;
}

/*
*1.decode http请求
*2.encode并 发送http响应
*/
struct http_response;
int http_onMessage(struct tcp_connection *tcpConnection) {
    printf("get message from %s\n", tcpConnection->name);
    
    /*读取并解析请求*/
    if (!tcpConnection->request)
        tcpConnection->request = http_request_new();

    if (!tcpConnection->response)
        tcpConnection->response = http_response_new();

    struct http_request *request = tcpConnection->request;
    http_request_parse(request, tcpConnection->input_buffer);
    

    struct HttpServer *httpServer = (struct HttpServer*)tcpConnection->data;
    
    struct http_response *response = tcpConnection->response;
    if(httpServer->requestCallback) 
        httpServer->requestCallback(request, response);

    /*编码发送并响应*/
    struct buffer *buffer = tcpConnection->output_buffer;
    http_encode_response(buffer, response);
    
    //发送http实体报文
    if (response->body) {
        buffer->file_base = response->body;
        buffer->file_size = response->file_size;
        
        response->body = NULL;
    }
    /*开启写事件*/
    tcp_connection_open_write_channel(tcpConnection);

    if (http_request_close_connection(request)) {
        http_request_free(request);
        http_response_free(response);
        tcpConnection->request = NULL;
        tcpConnection->response = NULL;
        tcp_connection_shutdown(tcpConnection);
    } else {
        http_request_reset(request);
        http_response_reset(response);
    }
    
    return 0;
}


//制定http版本 channel写回调函数
int http_handle_write(void *data) {
    struct tcp_connection *tcpConnection = (struct tcp_connection *)data;
    struct event_loop *eventLoop = tcpConnection->eventLoop;

    assert(eventLoop->owner_thread_id == pthread_self());

    struct buffer *output= tcpConnection->output_buffer;
    struct channel *chann =  tcpConnection->channel;
    
    struct iovec vec[2];
    vec[0].iov_base = output->data + output->readIndex;
    vec[0].iov_len = buffer_readable_size(output);
    vec[1].iov_base = output->file_base + output->file_offset;
    vec[1].iov_len = buffer_file_sendable_size(output);
    ember_debugx("header size = %d body_size = %d", vec[0].iov_len, vec[1].iov_len);
    int tmp = writev(chann->fd, vec, 2);
    ember_debugx("send %d bytes", tmp);
    if (tmp >= 0) {
        if (tmp >= vec[0].iov_len) {
            output->readIndex += vec[0].iov_len;
            output->file_offset += (tmp - vec[0].iov_len);
        } else {
            output->readIndex += tmp;
        }
    } else {
         if (errno == EAGAIN) {
            ember_msgx("TCP buffer is filled %s", tcpConnection->name);
            channel_write_event_enable(chann);
            return 0;
        }
        buffer_file_free(output);
        if (errno != EWOULDBLOCK) {
            if (errno == EPIPE || errno == ECONNRESET) {
                error(-1, errno, "write buffer occur error");       
            }
            return -1;
        }
    }

    if (buffer_readable_size(output) == 0 && 
                buffer_file_sendable_size(output) == 0){
        chann->data = tcpConnection->eventLoop;
        channel_write_event_disable(chann);
        chann->data= tcpConnection;
        
        buffer_file_free(output);

        if (tcpConnection->writeCompletedCallBack)
            tcpConnection->writeCompletedCallBack(tcpConnection);

        ember_debugx("send response finish");
    }
    return 0;
}

int http_handle_connection_established(void *data) {
    struct TCPServer *tcpServer = data;
    struct channel_map *map = tcpServer->eventLoop->channelMap;

    int connfd = handle_connection_established(data);
    struct channel *chann = map->entries[connfd];

    /*指定http数据发送*/
    chann->eventWriteCallback = http_handle_write;

    return 0;
}

struct HttpServer *create_http_server(struct event_loop *eventLoop, 
                int port, request_call_back requsetCallback, int nthread) {
    struct HttpServer *httpServer = malloc(sizeof(struct HttpServer));
    struct acceptor *acceptor = acceptor_init(port);
    httpServer->tcpServer = tcp_server_init(eventLoop, acceptor,
                        http_onConnectionCompleted, http_onConnectionClosed, 
                        http_onWriteCompleted, http_onMessage, nthread);

    httpServer->tcpServer->data = httpServer;
    httpServer->requestCallback = requsetCallback;

    return httpServer;
}

void http_server_start(struct HttpServer *httpServer) {
    tcp_server_start_with_read(httpServer->tcpServer, http_handle_connection_established);
}