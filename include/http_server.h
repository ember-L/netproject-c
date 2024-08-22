
#include "tcp_utils.h"
#include "http_request.h"
#include "http_response.h"
#ifndef __HTTP_SERVER_H
#define __HTTP_SERVER_H

typedef int(*request_call_back)(struct http_request*,struct http_response*);

struct HttpServer{
    struct TCPServer *tcpServer;
    /*提供给Http服务器处理 客户端请求 生成http响应*/
    request_call_back requestCallback;
};

struct HttpServer* create_http_server(struct event_loop *, int, request_call_back, int);

void http_server_start(struct HttpServer*);

#endif