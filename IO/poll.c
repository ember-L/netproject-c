#include "../utils.h"


#define SERVERPORT 6666
#define MAX_CLIENTS 16

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
    int listenfd = createTCPServer(SERVERPORT);
    fprintf(stdout,"open listening fd = %d\n",listenfd);
    
    struct pollfd client[MAX_CLIENTS];
    
    client[0].fd = listenfd;
    client[0].events = POLLRDNORM;
    for (int i = 1; i < MAX_CLIENTS; i++)
            client[i].fd = -1;

    int maxfd = 0;
    while(1){
        int nready = poll(client, maxfd+1, 1000);
        int i;
        if (client[0].revents & POLLRDNORM) {	/* new client connection */
			int connfd = acceptClient(listenfd);

			for (i = 1; i < MAX_CLIENTS; i++){
				if (client[i].fd < 0) {
					client[i].fd = connfd;	/* save descriptor */
					break;
				}
            }
			if (i == MAX_CLIENTS)
				exit(-1);

			client[i].events = POLLRDNORM;
			if (i > maxfd)
				maxfd = i;				/* max index in client[] array */

			if (--nready <= 0)
				continue;				/* no more readable descriptors */
		}
        for (i = 1; i <= maxfd; i++) {	/* check all clients for data */
			int clientfd;
            if ( (clientfd = client[i].fd) < 0)
				continue;
			if (client[i].revents & (POLLRDNORM | POLLERR)) {
				if(echo(clientfd) <= 0){
                    printf("client(fd=%d) disconnect\n", client[i].fd );
                    close( client[i].fd );
                    client[i].fd = 0;
                }
				if (--nready <= 0)
					break;				/* no more readable descriptors */
			}
        }
    }
    /*关闭监听描述符*/
    close(listenfd);
    return 0;
}