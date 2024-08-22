#ifndef __HTTP_REQUEST_H
#define __HTTP_REQUEST_H

#include "buffer.h"

#define INIT_HREADER_NUMBER 128

struct request_header{
    char *key;
    char *value;
};

enum http_parse_status {
    PARSE_START,
    PARSE_HREADER,
    PARSE_BODY,
    PARSE_DONE
};

struct http_request{
    char *method;
    char *url;
    char *version;

    struct request_header *httpHeaders;
    int headerNumber;
    int keepConnected;

    enum http_parse_status current_status;
};

struct http_request * http_request_new();

void http_request_free(struct http_request *);

void http_request_reset(struct http_request *);

int http_request_parse(struct http_request *,struct buffer *input);

int http_request_close_connection(struct http_request *httpRequest);

enum http_parse_status get_http_current_status(struct http_request*);

#endif