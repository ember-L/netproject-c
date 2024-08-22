#include "../include/http_server.h"
#include "../include/event_loop.h"
#include "../include/log.h"
#include "../utils.h"
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <dirent.h>

#define MAX_LINE 128

#define DYNAMIC_FILE    0
#define STATIC_FILE     1
#define DIR_FILE        2

const char *content_type = "Content-type";
const char *content_length = "Content-length";

void get_file_type (const char *filename, char *filetype){
    /*TODO 使用hash优化代码量*/
    if (strstr(filename, ".html"))
	    strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
	    strcpy(filetype, "image/gif");
    else if (strstr(filename, ".png"))
	    strcpy(filetype, "image/png");
    else if (strstr(filename, ".jpg"))
	    strcpy(filetype, "image/jpeg");
    else if (strstr(filename, ".ico"))
        strcpy(filetype, "image/x-icon");
    else if (strstr(filename, ".mp4"))
        strcpy(filetype, "image/mp4");
    else
	    strcpy(filetype, "text/plain");
}

int parse_url(const char *url, char *file_name){
    const char *root_name = "./root";
    /*默认请求home.html*/
    strcpy(file_name, root_name);
    strcat(file_name, url);
    if(strlen(url) == 1 && strcmp(url, "/") == 0) {
        // strcat(file_name, "home.html");
        return DIR_FILE;
    } else if (url[strlen(url) - 1] == '/'){
        /*目录*/
        return DIR_FILE;
    } 
    return STATIC_FILE;
}


void hanlde_error(struct http_response *response){
    int fd = open("./root/404.html", O_RDONLY, 0);
    struct stat file_stat;
    if(fstat(fd, &file_stat) < 0) {
        ember_debugx("error 404 file no found");
    }
    response->body = mmap(0, file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    response->file_size = file_stat.st_size;
    close(fd);
}

int handle_dir(char *file_name) {
    DIR *dp;
    struct dirent *dirp;

    dp = opendir(file_name);

    while((dirp = readdir(dp))) {
        ember_msgx("%s", dirp->d_name);
    }
    closedir(dp);
    return 0;
}

int handle_static_request(struct http_response *response, const char *file_name){
    struct stat sbuf;
    ember_debugx("file_name = %s",file_name);
    if (stat(file_name, &sbuf) < 0) {
        response->statusCode = NotFound;
        strcpy(response->statusMessage,"Not Found");
        response->keep_connected = 1;
        hanlde_error(response);
        return -1;
    }

    if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
        response->statusCode = Forbidden;
        strcpy(response->statusMessage,"Forbidden");
        return -1;
    }

    char *file_type = malloc(MAX_LINE);
    get_file_type(file_name, file_type);
    http_response_add_header(response, content_type, file_type);

    response->file_size = sbuf.st_size;
    char *filesize = malloc(MAX_LINE);
    sprintf(filesize, "%ld", sbuf.st_size);
    http_response_add_header(response, content_length, filesize);
    
    int sfd = open(file_name, O_RDONLY, 0);
    response->body = mmap(0, sbuf.st_size, PROT_READ, MAP_PRIVATE, sfd, 0);
    close(sfd);

    return 0;
}

/*负责生成 request*/
int onRequest(struct http_request *request, struct http_response *response){
    /*解析方法*/
    if (strcmp(request->method,"GET") != 0) {
        response->statusCode = NotImplemented;
        strcpy(response->statusMessage, "Not Implemented");
        return 0;
    }
    
    char filename[MAX_LINE];
    int file_type = parse_url(request->url, filename);
    if (file_type == STATIC_FILE) {
        if(handle_static_request(response, filename) == 0) {
            response->statusCode = OK;
            strcpy(response->statusMessage, "OK");
        }
    } else if (file_type == DIR_FILE){
        // if(handle_dir(filename) == 0){
        //     response->statusCode = OK;
        //     strcpy(response->statusMessage, "OK");
        // }
        strcat(filename, "home.html");
        handle_static_request(response, filename);
    }

    return 0;
}

int main(){
    struct event_loop* eventLoop = event_loop_init();

    struct http_server *httpServer = create_http_server(eventLoop, 
                                                SERVER_PORT, onRequest, 0);
    
    http_server_start(httpServer);

    event_loop_run(eventLoop);
}