#ifndef __THREAD_POOL_H
#define __THREAD_POOL_H

#include "event_loop.h"
#include "event_loop_thread.h"

struct thread_pool {
    struct event_loop *mainLoop;

    struct event_loop_thread *eventLoopThreads;

    int started;

    int nthread;

    //定位服务
    int position;
};

struct thread_pool *thread_pool_new(struct event_loop *,int);

void thread_pool_start(struct thread_pool *);

struct event_loop *thread_pool_get_loop(struct thread_pool *);

#endif