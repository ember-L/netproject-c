#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <error.h>
#include <errno.h>

#define BUF_LEN  100

int main(void)
{
    int clientfd;
    char send_buf[BUF_LEN],recv_buf[BUF_LEN+1];
    struct sockaddr server_addr;
    socklen_t addr_size = 0;
    struct sockaddr_in  server_sock_addr;
    
    /* 创建客户端socket */
    if (-1 == (clientfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)))
    {
        printf("socket error!\n");
        exit(1);
    }
    
    /* 向服务器发起请求 */
    memset(&server_sock_addr, 0, sizeof(server_sock_addr));  
    server_sock_addr.sin_family = AF_INET;
    inet_pton(AF_INET,"127.0.0.1",&server_sock_addr.sin_addr);
    server_sock_addr.sin_port = htons(6666); // 6666为服务端开启的端口
    
    addr_size = sizeof(server_addr);

    socklen_t server_len = sizeof(server_addr);
    struct sockaddr *reply_addr;
    reply_addr = malloc(server_len);
    socklen_t len;

    while (1)
    {
        memset(send_buf, 0, BUF_LEN);   // 重置缓冲区
        memset(recv_buf, 0, BUF_LEN);   // 重置缓冲区

        fputs("Send to server> ",stdout);
        fgets(send_buf,BUF_LEN,stdin);
        int i = strlen(send_buf);
        if(send_buf[i-1] == '\n')
            send_buf[i-1] = 0;
        /* 发送数据到服务端 */
        size_t s_sz = sendto(clientfd, send_buf, strlen(send_buf), 0, 
                (struct sockaddr*)&server_sock_addr, addr_size);
        
        if(s_sz < 0)
            error(1, errno, "sendto failed");
        printf("send bytes:%zu \n",s_sz);
        /* 接受服务端的返回数据 */
        len = 0;
        size_t r_sz = recvfrom(clientfd, recv_buf, BUF_LEN, 0, reply_addr, &len);
 
        if(r_sz < 0)
            error(1,errno,"recvfrom failed"); 
        recv_buf[r_sz] = 0;

        fputs(recv_buf,stdout);
        fputs("\n",stdout);
    }
    
    close(clientfd);   // 关闭套接字
 
    return 0;
}