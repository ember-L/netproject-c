#include "../utils.h"

const int BUF_LEN = 100;

int main(void)
{
    /* 配置 Server Sock 信息。*/
    struct sockaddr_in srv_sock_addr;
    memset(&srv_sock_addr, 0, sizeof(srv_sock_addr));
    srv_sock_addr.sin_family = AF_INET;
    srv_sock_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    srv_sock_addr.sin_port = htons(6666);

    int clientfd = 0;
    char send_buff[BUF_LEN];
    char recv_buff[BUF_LEN];

    /* 创建 Client Socket。*/
    if ((clientfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
        error(1,errno,"socket failed");
    }

    /* 连接到 Server Sock 信息指定的 Server。*/
    if (connect(clientfd,
                        (struct sockaddr *)&srv_sock_addr,
                        sizeof(srv_sock_addr)) == -1){
        error(1,errno,"connect failed");
    }

    /* 永循环从终端接收输入，并发送到 Server。*/
    while (1) {
        /* 从 stdin 接收输入，再发送到建立连接的 Server Socket。*/
        fputs("Send to server> ", stdout);
        memset(send_buff, 0, BUF_LEN);
        fgets(send_buff, BUF_LEN, stdin);
        send(clientfd, send_buff, BUF_LEN, 0);

        /* 从建立连接的 Server 接收数据。*/
        memset(recv_buff, 0, BUF_LEN);
        int n = 0;
        if((n = recv(clientfd, recv_buff, BUF_LEN,0)) <= 0){
            fprintf(stderr,"read error\n");
            break;
        }
        printf("Recevice from server: %s\n", recv_buff);
    }
    /* 每次 Client 请求和响应完成后，关闭连接。*/
    close(clientfd);

    return 0;
}