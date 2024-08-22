#include "../include/tcp_utils.h"
#include "../include/tcp_connection.h"
#include "../include/thread_pool.h"
#include "../include/acceptor.h"
#include "../include/log.h"
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <assert.h>
#include <sys/socket.h>
#include <unistd.h>

/* Set the specified socket in non-blocking mode, with no delay flag. */
int socketSetNonBlockNoDelay(int fd) {
    int flags, yes = 1;

    /* Set the socket nonblocking.
     * Note that fcntl(2) for F_GETFL and F_SETFL can't be
     * interrupted by a signal. */
    if ((flags = fcntl(fd, F_GETFL)) == -1) 
        return -1;
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
        return -1;

    /* This is best-effort. No need to check for errors. */
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(yes));
    return 0;
}

int createNonblockTCPServer(int port){
    /*配置Server Sock信息*/
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    
    //INADDR_ANY = 0.0.0.0监听本机所有地址
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    /*创建Server Socket*/
    int srvfd = 0;
    if((srvfd = socket(AF_INET,SOCK_STREAM,0))== -1){
        printf("Create socket file descriptor ERROR\n");
        exit(-1);
    }

    int yes = 1;
    //SO_REUSEADDR防止TIME_WAIT问题
    setsockopt(srvfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));// Best effort.

    if(bind(srvfd,
            (struct sockaddr*)&server_addr,
            sizeof(server_addr))==-1)
    {
        printf("Bind socket ERROR.\n");
        exit(-1);
    }

    if(listen(srvfd,10) == -1){
        printf("Listen socket ERROR.\n");
        exit(-1);     
    }
    
    socketSetNonBlockNoDelay(srvfd);
    return srvfd;
}

int createTCPServer(int port){
    /*配置Server Sock信息*/
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    
    //INADDR_ANY = 0.0.0.0监听本机所有地址
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    /*创建Server Socket*/
    int serverfd = 0;
    if((serverfd = socket(AF_INET,SOCK_STREAM,0))== -1){
        printf("Create socket file descriptor ERROR\n");
        exit(-1);
    }

    int yes = 1;
    //SO_REUSEADDR防止TIME_WAIT问题，参考smallchat
    setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    
    if(bind(serverfd,
            (struct sockaddr*)&server_addr,
            sizeof(server_addr))==-1)
    {
        printf("Bind socket ERROR.\n");
        exit(-1);
    }

    if(listen(serverfd,10) == -1){
        printf("Listen socket ERROR.\n");
        exit(-1);     
    }
    return serverfd;
}

int acceptClient(int listenfd){
    /*初始化Client socket 信息*/
    struct sockaddr_storage client_addr;
    bzero(&client_addr, sizeof(client_addr));
    socklen_t clientlen = sizeof(client_addr);
    int connfd = 0;

    if((connfd = accept(listenfd,
                    (struct sockaddr*)(&client_addr),
                    (socklen_t *)&clientlen)) == -1)
    {
        printf("Accept connection from client ERROR.\n");
        return -1;   
    }

    return connfd;
}

int timeoutCallback(void *data){
    struct tcp_connection *connection = data;
    handle_connection_closed(connection);
    return 0;
}

/*统一事件源*/
int sig_pipe[2];

void signal_handler(int sig){
    int old_errno = errno;
    send(sig_pipe[0], (char*)&sig, 1, 0);
    errno = old_errno;
}

void add_signal(int sig){
    struct sigaction sa;
    memset(&sa,0,sizeof(sa));
    sa.sa_handler = signal_handler;
    sa.sa_flags |= SA_RESTART;
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
}

int handle_signal(void *data){
    struct event_loop *eventLoop = data;
    char signals[1024];
    int n = recv(sig_pipe[1], &signals, sizeof(signals), 0);
    if (n <= 0)
        return -1;
    
    for(int i = 0; i < n; i++) {
        switch(signals[i]){
            case SIGCHLD:
            case SIGHUP:
            {
                continue;
            }
            case SIGTERM:
            case SIGINT:
            {
                ember_debugx("quit TCPServer");
                eventLoop->quit = 1;
                break;
            }
            case SIGALRM: 
            {
                eventLoop->timeout = 1;
                break;
            }
        }
    }
    return 0;
}


struct TCPServer *
tcp_server_init(struct event_loop *eventLoop,struct acceptor *acceptor,
                connection_completed_call_back connectionCompletedCallBack,
                connection_closed_call_back connectionClosedCallBack,
                write_completed_call_back writeCompletedCallBack,
                message_call_back messageCallBack,
                int nthread) {
    struct TCPServer *tcpServer = malloc(sizeof(struct TCPServer));
    
    tcpServer->eventLoop = eventLoop;
    tcpServer->acceptor = acceptor;
    
    tcpServer->connectionCompletedCallBack = connectionCompletedCallBack;
    tcpServer->connectionClosedCallBack = connectionClosedCallBack;
    tcpServer->writeCompletedCallBack = writeCompletedCallBack;
    tcpServer->messageCallBack = messageCallBack;

    tcpServer->nthread = nthread;
    tcpServer->threadPool = thread_pool_new(eventLoop, nthread);
    tcpServer->data = NULL;
    
    /*设置maineventLoop 信号接收管道*/
    if(socketpair(AF_UNIX, SOCK_STREAM, 0, sig_pipe))
        error(1,errno,"socketpair error");
    ember_debugx("TCPServer Create signal pipe");
    struct channel *chann1 = channel_new(sig_pipe[1], EVENT_READ, 
                            handle_signal, NULL, eventLoop);
    event_loop_add_channel_event(eventLoop, chann1);

    add_signal(SIGINT);
    add_signal(SIGTERM);
    /* 添加ALRM型号 */
    add_signal(SIGALRM);
    alarm(TIMESLOT);

    eventLoop->timerList = create_sort_timer_lst();
    eventLoop->timeout = 0;

    return tcpServer;
}

int handle_connection_established(void *data) {
    struct TCPServer *tcpServer = (struct TCPServer*)data;
    struct acceptor *acceptor = tcpServer->acceptor;
    int listen_fd = acceptor->listen_fd;

    int connfd = acceptClient(listen_fd);
    socketSetNonBlockNoDelay(connfd);

    ember_msgx("new connection established ,connect_fd = %d",connfd);

    //get sub-eventloop from the thread pool
    struct event_loop *eventLoop = thread_pool_get_loop(tcpServer->threadPool);

    struct tcp_connection *tcpConnection = tcp_connection_new(connfd, eventLoop, 
                                            tcpServer->connectionCompletedCallBack, 
                                            tcpServer->connectionClosedCallBack, 
                                            tcpServer->messageCallBack, 
                                            tcpServer->writeCompletedCallBack);
    //tcpConnection指向httpServer便于使用回调函数
    if (tcpServer->data)
        tcpConnection->data = tcpServer->data;

    /*添加定时器*/
    struct timer *t = generate_timer(timeoutCallback, tcpConnection);
    tcpConnection->timer = t;
    add_timer(tcpServer->eventLoop->timerList, t);

    return connfd;
}

/*提供给应用层 定制*/
void tcp_server_start_with_read(struct TCPServer *tcpServer, event_read_callback erc) {
    struct acceptor *acceptor = tcpServer->acceptor;
    struct event_loop *eventLoop = tcpServer->eventLoop;

    //开启线程
    thread_pool_start(tcpServer->threadPool);

    //开启acceptor channel，接收客户端连接，channel的data为tcpServer
    struct channel *chann = channel_new(acceptor->listen_fd, EVENT_READ, 
                                    erc, NULL, tcpServer);
    ember_debugx("open_listening fd == %d",acceptor->listen_fd);
    /*将acceptor绑定到 mainLoop上*/
    event_loop_add_channel_event(eventLoop, chann);
}

void tcp_server_start(struct TCPServer *tcpServer) {
    tcp_server_start_with_read(tcpServer, handle_connection_established);
}