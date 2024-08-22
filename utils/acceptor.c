#include "../include/acceptor.h"
#include "../include/tcp_utils.h"

struct acceptor* acceptor_init(int port) {
    struct acceptor *acceptor = malloc(sizeof(struct acceptor));
    
    acceptor->listen_fd = createNonblockTCPServer(port);
    acceptor->listen_port = port;

    return acceptor;
}