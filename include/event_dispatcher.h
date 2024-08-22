#ifndef __EVENT_DISPATCHER_H
#define __EVENT_DISPATCHER_H

#include "channel.h"

/*抽象应该事件分发器 用于select 、 poll、epoll的IO复用*/
struct event_dispatcher {
    /*对应实现*/
    const char*     name;

    /*初始化函数*/
    void *(*init)(struct event_loop *);

    /*通知dispatcher添加channel事件*/
    int(*add)(struct event_loop *, struct channel *);
    /*通知dispatcher删除channel事件*/
    int(*del)(struct event_loop *, struct channel *);
    /*通知dispatcher更新channel事件*/
    int(*update)(struct event_loop *, struct channel *);

    int (*dispatch)(struct event_loop *, struct timeval *);

    void (*clear)(struct event_loop *);
};

#endif