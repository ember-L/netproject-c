#include "../include/http_request.h"
#include "../include/log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


const char *HTTP1_0 = "HTTP/1.0";
const char *HTTP1_1 = "HTTP/1.1";
const char *KEEP_ALIVE = "keep-Alive";
const char *CLOSE = "close";

#define MAX_LINE 128

struct http_request *http_request_new() {
    struct http_request *request = malloc(sizeof(struct http_request));

    request->httpHeaders = malloc(sizeof(struct request_header) * INIT_HREADER_NUMBER);
    request->headerNumber = 0;
    request->keepConnected = 0;
    request->method = malloc(MAX_LINE);
    request->url = malloc(MAX_LINE);
    request->version = malloc(MAX_LINE);

    return request;
}

void http_request_free(struct http_request *request){
    if(request->httpHeaders){
        for(int i = 0; i < request->headerNumber; i++) {
            free(request->httpHeaders[i].key);
            free(request->httpHeaders[i].value);
        }
        free(request->httpHeaders);
        request->headerNumber = 0;
    }
    free(request->method);
    free(request->url);
    free(request->version);
    free(request);
}

void http_request_reset(struct http_request *request){
    if(request->httpHeaders){
        for(int i = 0; i < request->headerNumber; i++) {
            free(request->httpHeaders[i].key);
            request->httpHeaders[i].key = NULL;
            free(request->httpHeaders[i].value);
            request->httpHeaders[i].value = NULL;
        }
        request->headerNumber = 0;
    }
    memset(request->method, 0, MAX_LINE);
    memset(request->url, 0, MAX_LINE);
    memset(request->version, 0, MAX_LINE);
}

char *http_request_key2value(struct http_request *request, char *key){
    for(int i = 0; i < request->headerNumber; i++) {
        if (strncmp(key, request->httpHeaders[i].key, strlen(key))) {
            return request->httpHeaders[i].value;
        }
    }
    return NULL;
}

void http_request_add_header(struct http_request *request, char *key, char *value){
    request->httpHeaders[request->headerNumber].key = key;
    request->httpHeaders[request->headerNumber].value = value;
    request->headerNumber++;
}

int parse_start_line(struct http_request *request, struct buffer *input){
    char* start = input->data + input->readIndex;
    char *end = buffer_find_CRLF(input);
    if(!end)
        return 0;
    
    ssize_t start_line_size = end - start;
    
    /* method */
    char *space;
    if((space = strchr(start,' '))) {
        int method_size = space - start;
        memcpy(request->method, start, method_size);
        request->method[method_size] = 0;
    }

    /*url*/
    start = space + 1;    
    if((space = strchr(start, ' '))){
        int url_size = space - start;
        memcpy(request->url, start, url_size);
        request->url[url_size] = 0;
    }

    /*version*/
    start = space + 1;
    int version_size = end - start;
    memcpy(request->version, start, version_size);
    request->version[version_size] = 0;

    ember_debugx("request: %s %s %s", request->method, request->url, request->version);
    
    return start_line_size;
}

extern char *CRLF;
/*TODO定时器*/
int parse_headers(struct http_request *request, struct buffer *input){
    while(request->current_status == PARSE_HREADER) {
        char *end = buffer_find_CRLF(input);
        char *start = input->data + input->readIndex;

        ssize_t head_size = end - start;
        /*header读取完毕*/
        if(head_size == 0 || !end) {
            request->current_status = PARSE_BODY;
            break;
        }

        char *colon = strstr(start, ": ");

        ssize_t key_size = colon - start;
        char *key = malloc(key_size + 1);
        memcpy(key, start, key_size);
        key[key_size] = 0;

        ssize_t value_size = end - (colon + 2);
        char *value = malloc(value_size + 1);
        memcpy(value, colon + 2, value_size);
        value[value_size] = 0;
        // ember_debugx("%s: %s", key, value);
        
        http_request_add_header(request, key, value);
        input->readIndex += head_size + 2;
    }
    return request->headerNumber;
}

int http_request_parse(struct http_request *request, struct buffer *input){
    request->current_status = PARSE_START;
    /*解析起始行*/
    int start_line_size = parse_start_line(request, input);
    if (start_line_size){
        input->readIndex += start_line_size;
        input->readIndex += 2; //crlf
        request->current_status = PARSE_HREADER;
    }
    /*解析标头*/
    int head_number = parse_headers(request, input);
    if (head_number > 0){
        input->readIndex += 2;
    }
    /*TODO:解析body*/
    request->current_status = PARSE_DONE;

    return 0;
}

enum http_parse_status get_http_current_status(struct http_request *request){
    return request->current_status;
}

//根据request请求判断是否需要关闭服务器-->客户端单向连接
int http_request_close_connection(struct http_request *httpRequest) {
    char *connection = http_request_key2value(httpRequest, "Connection");

    if (connection != NULL && strncmp(connection, CLOSE, strlen(CLOSE)) == 0) {
        return 1;
    }

    if (httpRequest->version != NULL &&
        strncmp(httpRequest->version, HTTP1_0, strlen(HTTP1_0)) == 0 &&
        strncmp(connection, KEEP_ALIVE, strlen(KEEP_ALIVE)) != 0) {
        return 1;
    }
    return 0;
}