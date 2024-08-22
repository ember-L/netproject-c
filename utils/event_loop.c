#include "../include/event_loop.h"
#include "../include/event_dispatcher.h"
#include "../include/channel_map.h"
#include "../include/log.h"
#include <assert.h>
#include <pthread.h>

//io thread 添加channel事件至channel事件链表
int event_loop_handle_pending_channel(struct event_loop *eventLoop) {
    Lock(&eventLoop->mutex);
    eventLoop->is_handle_pending = 1;

    struct channel_list *channelElement = eventLoop->pending_head;

    while (channelElement) {
        //save into event_map
        struct channel *channel = channelElement->channel;
        int fd = channel->fd;
        ember_debugx("handling pending channle->fd == %d", fd);
        /*调用dispatch 注册事件*/
        if (channelElement->type == PENDING_ADD) 
            event_loop_handle_pending_add(eventLoop, channel);
        else if (channelElement->type == PENDING_REMOVE) 
            event_loop_handle_pending_remove(eventLoop, channel);
        else if (channelElement->type == PENDING_UPDATE) 
            event_loop_handle_pending_update(eventLoop, channel);
        
        channelElement = channelElement->next;
    }
    // 清空等待队列
    eventLoop->pending_head = eventLoop->pending_tail = NULL;
    eventLoop->is_handle_pending = 0;

    Unlock(&eventLoop->mutex);

    return 0;
}

/*添加channel*/
void event_loop_channel_buffer_noblock(struct event_loop *eventLoop, struct channel* channel, int type){
    //add channel into the pending list
    struct channel_list *channelElement = malloc(sizeof(struct channel_list));

    channelElement->channel = channel;
    channelElement->type = type;
    channelElement->next = NULL;

    if (eventLoop->pending_head == NULL) {
        eventLoop->pending_head = eventLoop->pending_tail = channelElement;
        eventLoop->pending_tail = channelElement;
    } else {
        eventLoop->pending_tail->next = channelElement;
        eventLoop->pending_tail = channelElement;
    }
}

/*唤醒event_loop并处理事件*/
int event_loop_do_channel_event(struct event_loop *eventLoop, struct channel *channel, int type) {
    Lock(&eventLoop->mutex);

    assert(eventLoop->is_handle_pending == 0);
    event_loop_channel_buffer_noblock(eventLoop, channel, type);

    Unlock(&eventLoop->mutex);

    if (eventLoop->owner_thread_id != pthread_self()) {
        ember_debugx("%s is not owner", eventLoop->thread_name);
        //主线程通过管道唤醒子线程，也就是解除子线程epoll_wait阻塞
        event_loop_wakeup(eventLoop);
    } else {
        //处理等待中的channel事件
        event_loop_handle_pending_channel(eventLoop);
    }
    return 0;
}

int event_loop_add_channel_event(struct event_loop *eventLoop ,struct channel *channel) {
    return event_loop_do_channel_event(eventLoop,  channel,  PENDING_ADD);
}

int event_loop_remove_channel_event(struct event_loop *eventLoop ,struct channel *channel) {
    return event_loop_do_channel_event(eventLoop,  channel,  PENDING_REMOVE);
}

int event_loop_update_channel_event(struct event_loop *eventLoop ,struct channel *channel) {
    return event_loop_do_channel_event(eventLoop, channel,  PENDING_UPDATE);
}

/*添加待处理channel至channel表*/
int event_loop_handle_pending_add(struct event_loop *eventLoop, struct channel *channel) {
    ember_msgx("add channel fd == %d, %s", channel->fd ,eventLoop->thread_name);
    struct channel_map *map = eventLoop->channelMap;

    if(channel->fd < 0)  return 0;

    if (channel->fd >= map->number) {
        if (map_make_space(map, channel->fd, sizeof(struct channel*)) == -1)
            return -1;
    }

    if (map->entries[channel->fd] == NULL) {
        map->entries[channel->fd] = channel;

        //add channel
        struct event_dispatcher *eventDispatcher = eventLoop->eventDispatcher;
        eventDispatcher->add(eventLoop, channel);
        return 1;
    }

    return 0;
}

int event_loop_handle_pending_remove(struct event_loop *eventLoop, struct channel *channel) {
    struct channel_map *map = eventLoop->channelMap;
    ember_msgx("remove channel fd == %d, %s",channel->fd ,eventLoop->thread_name);

    if(channel->fd < 0) return 0;

    if(channel->fd >= map->number) return -1;

    struct channel *chann = map->entries[channel->fd]; 

    struct event_dispatcher *eventDispatcher = eventLoop->eventDispatcher;

    int retval = 0;

    if (eventDispatcher->del(eventLoop, chann)){
        retval = -1;
    } else {
        retval = 1;
    }

    map->entries[channel->fd] = NULL;

    return retval;
}

int event_loop_handle_pending_update(struct event_loop *eventLoop, struct channel *channel){
    ember_msgx("update channel fd == %d, %s",channel->fd ,eventLoop->thread_name);
    
    struct channel_map *map = eventLoop->channelMap;
    assert(map->number >= channel->fd);

    if (channel->fd < 0) return 0;

    if (map->entries[channel->fd] == NULL) 
        return -1;

    struct event_dispatcher *eventDispatcher = eventLoop->eventDispatcher;

    eventDispatcher->update(eventLoop, channel);
}

/*event_dispatcher 调用*/
int channel_event_activate(struct event_loop *eventLoop, int fd, int revents) {
    struct channel_map *map = eventLoop->channelMap;
    ember_msgx("activate channel fd == %d ,revents = %d , %s", fd ,revents, eventLoop->thread_name);

    if(fd < 0)
        return 0;

    if(fd >= map->number) return -1;

    struct channel *channel = map->entries[fd];
    assert(fd == channel->fd);
    // tcp_connection回调函数 接收数据或发送数据
    if (revents & (EVENT_READ)) {
        if (channel->eventReadCallback) 
            channel->eventReadCallback(channel->data);
    }
    if (revents & (EVENT_WRITE)) {
        if (channel->eventWriteCallback) 
            channel->eventWriteCallback(channel->data);
    }
    return 0;
}

void event_loop_wakeup(struct event_loop *eventLoop) {
    char one = 'a';

    ssize_t n = write(eventLoop->socketPair[0], &one, sizeof(one)) ;

    if(n != sizeof(one)) {
        LOG_ERR("wake_up event loop thread failed");
    }
    ember_debugx("handling wake up, %s", eventLoop->thread_name);
}

int handleWakeup(void *data){
    struct event_loop *eventLoop = (struct event_loop*)data;

    char one;
    ssize_t  n = read(eventLoop->socketPair[1], &one, sizeof(one));
    if(n != sizeof one) {
        LOG_ERR("handleWakeup  failed");
    }

    ember_debugx("wakeup, %s", eventLoop->thread_name);
    return 0;
}


struct event_loop* event_loop_init(){
    return event_loop_init_with_name(NULL);
}

struct event_loop* event_loop_init_with_name(char* thread_name){
    struct event_loop *eventLoop = malloc(sizeof(struct event_loop));
    pthread_mutex_init(&eventLoop->mutex,NULL);
    pthread_cond_init(&eventLoop->cond,NULL);

    if(thread_name)
        eventLoop->thread_name = thread_name;
    else
        eventLoop->thread_name = "main thread";
    
    eventLoop->quit = 0;
    eventLoop->channelMap = malloc(sizeof(struct channel_map));
    map_init(eventLoop->channelMap);

    ember_msgx("set epoll as dispatcher, %s", eventLoop->thread_name);
    eventLoop->eventDispatcher = &epoll_dispatcher;

    eventLoop->event_dispatcher_data = eventLoop->eventDispatcher->init(eventLoop);

    //add the sockefd to event
    eventLoop->owner_thread_id = pthread_self();
    if(socketpair(AF_UNIX, SOCK_STREAM, 0, eventLoop->socketPair)) {
        error(1,errno,"socketpair error");
    }

    eventLoop->is_handle_pending = 0;
    eventLoop->pending_head = eventLoop->pending_tail = NULL;

    struct channel *channel = channel_new(eventLoop->socketPair[1],
                                EVENT_READ, handleWakeup, NULL, eventLoop);

    event_loop_add_channel_event(eventLoop, channel);

    eventLoop->timeout = 0;
    eventLoop->timerList = NULL;
    
    return eventLoop;
}

/*定时器超时处理函数*/
void handle_timeout(void * data){
    struct event_loop *evenLoop = data;
    assert(evenLoop->timerList != NULL);
    tick(evenLoop->timerList);
    /*一次alarm调用只会引起一次SIGALRM信号，需要重新启动*/
    alarm(TIMESLOT);
}

 int event_loop_run(struct event_loop *eventLoop) {
    assert(eventLoop != NULL);

    struct event_dispatcher *dispatcher = eventLoop->eventDispatcher;

    if (eventLoop->owner_thread_id  != pthread_self())
        exit(1);

    struct timeval tm;

    tm.tv_sec = 1;
    ember_msgx("eventloop run[%s]", eventLoop->thread_name);
    while(!eventLoop->quit) { 
        /*处理IO事件，阻塞在epoll_wait处，使用管道唤醒*/
        dispatcher->dispatch(eventLoop, &tm);
        if (eventLoop->timeout) {
            handle_timeout(eventLoop);
            eventLoop->timeout = 0;
        }
        event_loop_handle_pending_channel(eventLoop);
    }
    ember_msgx("eventloop end [%s] ", eventLoop->thread_name);
    return 0;
 }