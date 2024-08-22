#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <errno.h>
#include <error.h>

static int count;
static void recvfrom_int(int signo) {
printf("\nreceived %d datagrams\n", count);
    exit(0);
}

#define BUF_LEN  4096
#define MAXLINE 100
#define SERVER_PORT 6666


int createUDPServer(int port){
    int serverfd;
    struct sockaddr_in server_addr;
    
    /* 创建 UDP 服务端 Socket */
    if ( (serverfd = socket(AF_INET, SOCK_DGRAM, 0)) ==  -1)
    {
        error(1,errno,"socket failed");
    }
    
    /* 设置服务端信息 */
    bzero(&server_addr,sizeof(server_addr));  // 给结构体server_addr清零
    server_addr.sin_family = AF_INET;       // 使用IPv4地址
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // 自动获取IP地址
    server_addr.sin_port = htons(port);      // 端口

    
    /* 绑定操作，绑定前加上上面的socket属性可重复使用地址 */
    if (-1 == bind(serverfd, (struct sockaddr*)&server_addr, sizeof(server_addr)))
    {
        printf("bind error!\n");
        exit(1);
    }

    return serverfd;
}


int main(void)
{
    int serverfd = createUDPServer(SERVER_PORT);

    /*初始化Client socket 信息*/
    struct sockaddr_storage client_addr;
    bzero(&client_addr, sizeof(client_addr));
    socklen_t clientlen = sizeof(client_addr);
    char client_hostname[MAXLINE] , client_port[MAXLINE];

    char send_buf[BUF_LEN],recv_buf[BUF_LEN];

    signal(SIGINT, recvfrom_int);

    while (1){
        
        /* 清空缓冲区 */
        memset(send_buf, 0, BUF_LEN);  
        memset(recv_buf, 0, BUF_LEN);  

        /* 接受客户端的返回数据 */
        size_t r_sz = recvfrom(serverfd, recv_buf, BUF_LEN, 0,
                         (struct sockaddr*)&client_addr,&clientlen);
 
        if(r_sz < 0)
            error(1,errno,"recvfrom failed");

        /*将客户端套接字转换为主机名与服务名*/
        getnameinfo((struct sockaddr*)&client_addr, clientlen,client_hostname,
                 MAXLINE, client_port, MAXLINE, 0);
        printf("Recevice data from client(%s %s): %s\n",client_hostname,client_port, recv_buf);

        sprintf(send_buf,"Server> %s",recv_buf);
        printf("%s\n",send_buf);
        /* 发送数据到客户端 */
        sendto(serverfd, send_buf, strlen(send_buf), 0, (struct sockaddr*)&client_addr, clientlen);

        count++;
    }
    
    close(serverfd);

    return 0;
}