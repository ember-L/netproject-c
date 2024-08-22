#include "../include/event_dispatcher.h"
#include "../include/event_loop.h"
#include "../include/log.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>

#define MAXEVENTS 128

typedef struct{
    int                 event_count;
    int                 nfds;
    int                 realloc_copy;
    int                 efd;
    struct epoll_event  *events;
}epoll_dispatcher_t;

static void *epoll_init(struct event_loop *);
/*控制epoll行为函数*/
static int epoll_add(struct event_loop *, struct channel *);

static int epoll_del(struct event_loop *, struct channel *);

static int epoll_update(struct event_loop *, struct channel *);

static int epoll_dispatch(struct event_loop *, struct timeval *);

static void epoll_clear(struct event_loop *);

const struct event_dispatcher epoll_dispatcher = {
    "epoll",
    epoll_init,
    epoll_add,
    epoll_del,
    epoll_update,
    epoll_dispatch,
    epoll_clear,
};

void *epoll_init(struct event_loop *evenLoop) {
    epoll_dispatcher_t *epollDispatcher = malloc(sizeof(epoll_dispatcher_t));

    epollDispatcher->event_count = epollDispatcher->nfds =epollDispatcher->efd = 0;

    epollDispatcher->realloc_copy = 0;

    epollDispatcher->efd = epoll_create(1024);

    if(epollDispatcher->efd < 0)
        error(1, errno, "epoll create error");

    epollDispatcher->events = calloc(MAXEVENTS, sizeof(struct epoll_event));

    return epollDispatcher;
}

int epoll_add(struct event_loop *eventLoop, struct channel * channel){
    epoll_dispatcher_t *epollDispatcherData = (epoll_dispatcher_t*) eventLoop->event_dispatcher_data;

    int fd = channel->fd;
    int events = 0;
    if (channel->events & EVENT_READ) {
        events |= EPOLLIN;
    } 
    if (channel->events & EVENT_WRITE) {
        events |= EPOLLOUT;
    }

    struct epoll_event event;
    event.data.fd = fd;
    event.events = events;
    if(epoll_ctl(epollDispatcherData->efd, EPOLL_CTL_ADD, fd, &event) == -1){
        error(1, errno, "epoll_ctl add  fd failed");
    }
    return 0;
}

int epoll_del(struct event_loop *evenLoop, struct channel * channel) {
    epoll_dispatcher_t *epollDispatcherData = (epoll_dispatcher_t*) evenLoop->event_dispatcher_data;
    int fd = channel->fd;

    int events = 0;
    if (channel->events & EVENT_READ) {
        events |= EPOLLIN;
    }

    if (channel->events & EVENT_WRITE) {
        events |= EPOLLOUT;
    }

    struct epoll_event event;
    event.data.fd = fd;
    event.events = events;

    if(epoll_ctl(epollDispatcherData->efd, EPOLL_CTL_DEL, fd, &event)){
        error(1, errno, "epoll_ctl del  fd failed");
    }
    
    return 0;
}

int epoll_update(struct event_loop *evenLoop, struct channel * channel) {
    epoll_dispatcher_t *epollDispatcherData = (epoll_dispatcher_t*) evenLoop->event_dispatcher_data;
    int fd = channel->fd;

    int events = 1;
    if (channel->events & EVENT_READ) {
        events = events | EPOLLIN;
    }

    if (channel->events & EVENT_WRITE) {
        events = events | EPOLLOUT;
    }

    struct epoll_event event;
    event.data.fd = fd;
    event.events = events;

    if(epoll_ctl(epollDispatcherData->efd, EPOLL_CTL_MOD, fd, &event) == -1){
        error(1, errno, "epoll_ctl mod  fd failed");
    }

    return 0;
}

static int epoll_dispatch(struct event_loop *eventLoop, struct timeval *tm) {
    epoll_dispatcher_t *epollDispatcherData = eventLoop->event_dispatcher_data;
    int n = epoll_wait(epollDispatcherData->efd, epollDispatcherData->events, MAXEVENTS, -1);
    ember_msgx("epoll_wait wakeup, %s",eventLoop->thread_name);

    for(int i = 0;i < n; i++) {
        if ((epollDispatcherData->events[i].events & EPOLLERR) || 
            (epollDispatcherData->events[i].events & EPOLLHUP)){
            fprintf(stderr, "epoll error fd = %d\n", 
                            epollDispatcherData->events[i].data.fd);
            close(epollDispatcherData->events[i].data.fd);
            continue;
        }
        /*唤醒eventLoop*/
        if(epollDispatcherData->events[i].events & EPOLLIN) {
            ember_msgx("get msg channel fd == %d for read ,%s",epollDispatcherData->events[i].data.fd, eventLoop->thread_name);
            channel_event_activate(eventLoop, epollDispatcherData->events[i].data.fd, EVENT_READ);
        }
        if(epollDispatcherData->events[i].events & EPOLLOUT) {
            ember_msgx("put msg channel fd == %d for write ,%s",epollDispatcherData->events[i].data.fd, eventLoop->thread_name);
            channel_event_activate(eventLoop, epollDispatcherData->events[i].data.fd, EVENT_WRITE);
        }
    }

    return 0;
}

static void epoll_clear(struct event_loop *eventLoop) {
    epoll_dispatcher_t *epollDispatcherData = eventLoop->event_dispatcher_data;

    free(epollDispatcherData->events);
    close(epollDispatcherData->efd);
    free(epollDispatcherData);
    eventLoop->event_dispatcher_data = NULL;    
}