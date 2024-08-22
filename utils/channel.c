#include "../include/channel.h"
#include "../include/event_loop.h"

struct channel *
channel_new(int fd, int events, event_read_callback erc, event_write_callback ewc, void *data){
    struct channel* chann = malloc(sizeof(struct channel));
    chann->fd = fd;
    chann->events = events;
    chann->eventReadCallback = erc;
    chann->eventWriteCallback = ewc;
    chann->data = data;
    
    return chann;
}

int channel_write_event_is_enabled(struct channel *channel){
    return channel->events & EVENT_WRITE;
}

int channel_write_event_enable(struct channel *channel){
    struct event_loop *el = (struct event_loop*) channel->data;
    channel->events |= EVENT_WRITE;

    event_loop_update_channel_event(el, channel);

    return 0;
}

int channel_write_event_disable(struct channel *channel){
    struct event_loop *el = (struct event_loop*) channel->data;
    channel->events &= (~EVENT_WRITE);

    event_loop_update_channel_event(el, channel);

    return 0;
}