#include "../include/acceptor.h"
#include "../include/tcp_connection.h"
#include "../utils.h"
#include "../include/event_loop.h"

int onConnectionCompleted(struct tcp_connection *tcpConnection) {
    printf("connect completed!!!\n");
    return 0;
}

int onMessage(struct tcp_connection *tcpConnection) {
    printf("get message from %s\n", tcpConnection->name);
    printf("%s", tcpConnection->input_buffer->data);

    struct buffer *output = buffer_new();

    int size = buffer_readable_size(tcpConnection->input_buffer);
    for(int i = 0; i < size; i++) {
        buffer_append_char(output, buffer_read_char(tcpConnection->input_buffer));
    }
    tcp_connection_send_buffer(tcpConnection, output);
    return 0;
}

int onWriteCompleted(struct tcp_connection *tcpConnect) {
    printf("write commplet\n");
    return 0;
}

int onConnectionClosed(struct tcp_connection *tcpConnection) {
    printf("connection closed\n");
    return 0;
}

int main() {
    // 主线程event_loop
    struct event_loop *eventLoop = event_loop_init();

    //初始化acceptor
    struct acceptor *acceptor = acceptor_init(SERVER_PORT);
    //初始化tcp server
    struct TCPServer *tcpServer = tcp_server_init(eventLoop, acceptor,
                                    onConnectionCompleted, onConnectionClosed, 
                                    onWriteCompleted, onMessage, 2);
    tcp_server_start(tcpServer);

    event_loop_run(eventLoop);
}