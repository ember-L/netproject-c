#include "../include/acceptor.h"
#include "../include/tcp_connection.h"
#include "../utils.h"
#include "../include/log.h"
#include "../include/event_loop.h"
int onConnectionCompleted(struct tcp_connection *tcpConnection) {
    printf("connect completed!!!\n");
    return 0;
}

int onMessage(struct tcp_connection *tcpConnection) {
    printf("get message from %s\n", tcpConnection->name);
    struct buffer *input = tcpConnection->input_buffer;
    printf("%s", input->data);

    struct buffer *output = tcpConnection->output_buffer;

    int size = buffer_readable_size(input);
    for(int i = 0; i < size; i++) {
        buffer_append_char(output, buffer_read_char(input));
    }
    tcp_connection_open_write_channel(tcpConnection);
    ember_debugx("send over");
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
    // 创建 efd = 3 socket_pair[0] = 4  socket_pair[1] = 5 
    struct event_loop *eventLoop = event_loop_init();

    //初始化acceptor
    // listen_fd = 6
    struct acceptor *acceptor = acceptor_init(SERVER_PORT);
    //初始化tcp server
    struct TCPServer *tcpServer = tcp_server_init(eventLoop, acceptor,
                                    onConnectionCompleted, onConnectionClosed, 
                                    onWriteCompleted, onMessage, 0);
    tcp_server_start(tcpServer);

    event_loop_run(eventLoop);
}