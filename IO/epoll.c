#include "../utils.h"

#define SERVERPORT 6666
#define MAX_EVENTS 128
#define MAX_CLIENTS 16

const int MAXLINE = 100;
const int BUF_LEN = 100;

/*简单的echo实现*/
int echo(int clientfd){
    char buf[BUF_LEN];
    ssize_t recv_len = 0;
    // while(1){
        /*接收指定Client Socket发出的数据*/
    memset(buf,0,sizeof(buf));
    if((recv_len = recv(clientfd,buf,BUF_LEN,0))<= 0)
        return recv_len;
    
    printf("Recevice data from client: %s", buf);
    send(clientfd,buf,recv_len,0);
    // }
    return recv_len;
}

int main(int argc, char ** argv){
    int listenfd = createTCPServer(SERVERPORT);
    fprintf(stdout,"open listening fd = %d\n",listenfd);
    
    int epfd = epoll_create(1024);
    struct epoll_event event,*events;
    event.events = EPOLLET | EPOLLIN;
    event.data.fd = listenfd;
    if(epoll_ctl(epfd,EPOLL_CTL_ADD,listenfd,&event) == -1){
        fprintf(stderr, "epoll_ctl add listen fd failed");
        exit(1);
    }

    events = calloc(MAX_EVENTS, sizeof(event));
    while(1){
        int nready = epoll_wait(epfd,events,MAX_EVENTS,1000);
        
        for(int i = 0 ;i < nready ;i++){
            if ((events[i].events & EPOLLERR) ||
                (events[i].events & EPOLLHUP) ||
                (!(events[i].events & EPOLLIN))) {
                fprintf(stderr, "epoll error\n");
                if(epoll_ctl(epfd,EPOLL_CTL_DEL,listenfd,&event) == -1){
                    fprintf(stderr, "epoll_ctl add connect fd failed");
                    exit(1);
                }
                close(events[i].data.fd);
                continue;
            } else if (events[i].data.fd == listenfd){
                int connfd = acceptClient(listenfd);
                event.data.fd = connfd;
                event.events = EPOLLIN | EPOLLET;
                if(epoll_ctl(epfd,EPOLL_CTL_ADD,connfd,&event) == -1){
                    fprintf(stderr, "epoll_ctl add connect fd failed");
                    exit(1);
                }
            } else {
                int clientfd = events[i].data.fd;
                if(echo(clientfd) <= 0){
                    printf("client(fd=%d) disconnect\n", clientfd);

                    if(epoll_ctl(epfd,EPOLL_CTL_DEL,clientfd,NULL) == -1){
                        fprintf(stderr, "epoll_ctl clientfd del fd failed");
                    }
                    close( clientfd );
                }
            }
        }
    }
    free(events);
    /*关闭监听描述符*/
    close(listenfd);
    return 0;
}