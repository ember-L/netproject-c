#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#include <arpa/inet.h>
#include <sys/socket.h>

#include <sys/types.h>
#include <sys/wait.h>

const int BUF_LEN = 100;
#define MAX_PROCESS 5

const char* send_buff = "hello!!!\n";

int createClient(){
    /* 配置 Server Sock 信息。*/
    struct sockaddr_in srv_sock_addr;
    memset(&srv_sock_addr, 0, sizeof(srv_sock_addr));
    srv_sock_addr.sin_family = AF_INET;
    srv_sock_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    srv_sock_addr.sin_port = htons(6666);

    int clientfd = 0;

    /* 创建 Client Socket。*/
    if ((clientfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
        printf("Create socket ERROR.\n");
        exit(EXIT_FAILURE);
    }

    /* 连接到 Server Sock 信息指定的 Server。*/
    if (connect(clientfd,
                        (struct sockaddr *)&srv_sock_addr,
                        sizeof(srv_sock_addr)) == -1){
        printf("Connect to server ERROR.\n");
        exit(EXIT_FAILURE);
    }


    return clientfd;
}

void sendMsg(int clientfd){
    char recv_buff[BUF_LEN];
    /* 永循环从终端接收输入，并发送到 Server。*/
    int cnt = 5;
    while (cnt--) {
        send(clientfd, send_buff, BUF_LEN, 0);

        /* 从建立连接的 Server 接收数据。*/
        memset(recv_buff, 0, BUF_LEN);
        int n = 0;
        if((n = recv(clientfd, recv_buff, BUF_LEN,0)) <= 0){
            printf("read error\n");
            break;
        }
        printf("Recevice[%d] %s\n",clientfd, recv_buff);
    }
    /* 每次 Client 请求和响应完成后，关闭连接。*/
    close(clientfd);
}

int main(void)
{

    pid_t pids[MAX_PROCESS];
    int clientfd = 0;
    clientfd = createClient();
    for(int i = 0;i < MAX_PROCESS;i++){
        pids[i] = fork();

        if(pids[i] == 0){
            clientfd = createClient();
            sendMsg(clientfd);
            exit(0);
        }
    }


    for(int i = 0;i < MAX_PROCESS; i++){
        waitpid(pids[i],0,0);
    }

    return 0;
}