#include "../utils.h"

#define SERVERPORT 6666
#define MAX_CLIENTS 10

const int MAXLINE = 100;
const int BUF_LEN = 100;

/*简单的echo实现*/
int echo(int clientfd){
    char buf[BUF_LEN];
    ssize_t recv_len = 0;

    /*接收指定Client Socket发出的数据*/
    memset(buf,0,sizeof(buf));
    if((recv_len = recv(clientfd,buf,BUF_LEN,0))<= 0){
        return recv_len;
    }
    printf("Recevice data from client: %s", buf);
    send(clientfd,buf,recv_len,0);
    return recv_len;
}

int main(int argc, char ** argv){
    int listenfd = createNonblockTCPServer(SERVERPORT);
    fprintf(stdout,"open listening fd = %d\n",listenfd);
    socketSetNonBlockNoDelay(listenfd); 
    int clientfds[MAX_CLIENTS] = {0};
    int max_clients = 0;
    int max_fd = 0;
   
    fd_set read_fds;
    FD_ZERO(&read_fds);
    // 添加监听描述符
    FD_SET(listenfd, &read_fds);
    // 添加标准输入描述符
    FD_SET(STDIN_FILENO, &read_fds);
    while(1){
        int retval = 0;
        
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        fd_set ready_fds = read_fds;

        max_fd = max_clients;
        if(max_fd < listenfd) max_fd = listenfd;

        retval = select(max_fd+1, &ready_fds, 0, 0, &tv);
        if (retval == -1) {
            perror("select() error");
            exit(1);
        } else if (retval) {
            /*监听描述符，添加连接描述符*/
            if(FD_ISSET(listenfd, &ready_fds)){
                int clientfd = acceptClient(listenfd);
                if(clientfd < 0) 
                    exit(-1);
                FD_SET(clientfd,&read_fds); //添加可读文件描述符
                clientfds[clientfd] = clientfd;
                if(clientfd > max_clients) 
                    max_clients = clientfd;
            }

            for(int i = 0 ;i <= max_clients;i++){
                if(clientfds[i] == 0) 
                    continue;
                
                if(FD_ISSET(i, &ready_fds)){
                    if(echo(clientfds[i]) <= 0){
                        printf("client(fd=%d) disconnect\n", clientfds[i]);
                        FD_CLR( clientfds[i], &read_fds);
                        close( clientfds[i]);
                        clientfds[i] = 0;
                    }
                    
                }
            }
            if(FD_ISSET(STDIN_FILENO, &ready_fds)){
                char std_buf[BUF_LEN];
                memset(std_buf, 0, BUF_LEN);
                fgets(std_buf,BUF_LEN,stdin);
                fprintf(stdout,"stdout> %s\n",std_buf);
            }
        }
    }
    /*关闭监听描述符*/
    close(listenfd);
    return 0;
}