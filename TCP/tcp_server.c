#include "../utils.h"

#define SERVERPORT 6666
#define MAXLINE 100
#define BUF_LEN 100

/*简单的echo实现*/
void echo(int clientfd){
    char buf[BUF_LEN];
    ssize_t recv_len = 0;
    while(1){
        /*接收指定Client Socket发出的数据*/
        memset(buf,0,sizeof(buf));
        if((recv_len = recv(clientfd,buf,BUF_LEN,0))<= 0){
            printf("disconnect client fd = %d\n",clientfd);
            /*处理完Client请求，关闭连接*/
            close(clientfd);
            break;
        }
        printf("Recevice data from client: %s", buf);
        /*发送接收到的信息给客户端*/
        send(clientfd,buf,recv_len,0);
    }
}



int main(int argc, char ** argv){
    int listenfd = createTCPServer(SERVERPORT);
    int clientfd = 0;
    
    while(1){
        if((clientfd = acceptClient(listenfd)) < 0){
            exit(-1);
        }
        echo(clientfd);
    }
    /*关闭监听描述符*/
    close(listenfd);
    return 0;
}