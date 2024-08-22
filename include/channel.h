#ifndef __CHANNEL_H
#define __CHANNEL_H

#define EVENT_TIMEOUT   0x01

#define EVENT_READ      0x02

#define EVENT_WRITE     0x04

#define EVENT_SIGNAL    0x08

#include "require.h"


typedef int (*event_read_callback)(void *data);

typedef int (*event_write_callback)(void *data);

struct channel {
    int                     fd;
    int                     events;  //the kind of event

    event_read_callback     eventReadCallback;
    event_write_callback    eventWriteCallback;
    /*callback data, 可能是event_loop，也可能是tcp_server或者tcp_connection*/
    void*                   data;
};

struct channel*
channel_new(int, int, event_read_callback, event_write_callback, void *);

int channel_write_event_is_enabled(struct channel *channel);
int channel_write_event_enable(struct channel *channel);
int channel_write_event_disable(struct channel *channel);

#endif