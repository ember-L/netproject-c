#include "../include/http_response.h"
#include "../include/log.h"
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#define INIT_RESPONSE_HEADER_SIZE 128
#define MAX_MESSAGE_SIZE 128

struct http_response *http_response_new(){
    struct http_response *response = malloc(sizeof(struct http_response));
    response->body = NULL;
    response->reseponseHeaders = malloc(sizeof(struct response_header) * INIT_RESPONSE_HEADER_SIZE);
    response->headerNumber = 0;
    response->statusCode = UnKnown;
    response->statusMessage = malloc(MAX_MESSAGE_SIZE);
    memset(response->statusMessage, 0, MAX_MESSAGE_SIZE);
    response->keep_connected = 0;
    return response;
}

void http_response_add_header(struct http_response * response,
                                        const char *key,char *value) {
    response->reseponseHeaders[response->headerNumber].key = key;
    response->reseponseHeaders[response->headerNumber].value = value;
    response->headerNumber++;
}

void http_response_reset(struct http_response *response){
    if(response->reseponseHeaders){
        for(int i = 0; i < response->headerNumber; i++) {
            response->reseponseHeaders[i].key = NULL;
            free(response->reseponseHeaders[i].value);
            response->reseponseHeaders[i].value = NULL;
        }
    }
    memset(response->statusMessage, 0, MAX_MESSAGE_SIZE);
    response->statusCode = UnKnown;
    response->headerNumber = 0;
}

void http_response_free(struct http_response *response){
    if(response->reseponseHeaders){
        for(int i = 0; i < response->headerNumber; i++) {
            response->reseponseHeaders[i].key = NULL;
            free(response->reseponseHeaders[i].value);
        }
        free(response->reseponseHeaders);
    }
    free(response->statusMessage);
    free(response);
}

void http_encode_response(struct buffer *output, struct http_response *response){
    char buf[32];

    sprintf(buf, "HTTP/1.1 %d ", response->statusCode);
    
    buffer_append_string(output, buf);
    buffer_append_string(output, response->statusMessage);
    buffer_append_string(output, "\r\n");

    buffer_append_string(output, "Server: tiny Web Server by Ember\r\n");

    if (response->keep_connected) {
        buffer_append_string(output, "Connection: close\r\n");
    } else {
        buffer_append_string(output, "Connection: Keep-Alive\r\n");
    }

    if (response->reseponseHeaders != NULL && response->headerNumber > 0) {
        for (int i = 0; i < response->headerNumber; i++) {
            buffer_append_string(output, response->reseponseHeaders[i].key);
            buffer_append_string(output, ": ");
            buffer_append_string(output, response->reseponseHeaders[i].value);
            buffer_append_string(output, "\r\n");
        }
    }
    buffer_append_string(output, "\r\n");
}