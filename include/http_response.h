#ifndef __HTTP_RERESPONSE_H
#define __HTTP_RERESPONSE_H
#include "buffer.h"

struct response_header{
    const char *key;
    char *value;
};

enum HttpStatusCode {
    UnKnown,
    OK = 200,
    BadRequest = 400,
    Forbidden = 403,
    NotFound = 404,
    NotImplemented = 501
};

struct http_response {
    enum HttpStatusCode statusCode;
    char *statusMessage;
    int keep_connected;

    /*响应body部分 文件映射*/
    char *body;
    int file_size;

    struct response_header *reseponseHeaders;
    int headerNumber;
};

struct http_response *http_response_new();
void http_response_free(struct http_response *response);
void http_response_reset(struct http_response *response);
void http_response_add_header(struct http_response *,const char *,char *);
void http_encode_response(struct buffer *output,struct http_response *httpResponse);

#endif