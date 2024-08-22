#ifndef __EVENT_LOOP_THREAD_H
#define __EVENT_LOOP_THREAD_H

#include <pthread.h>
#include "event_loop.h"

struct event_loop_thread{
    struct event_loop *eventLoop;

    pthread_t tid;
    pthread_mutex_t mutex;
    pthread_cond_t cond;

    char *thread_name;
    long thread_count; //connection handled
};

//初始化分配内存的event_loop_thread
int event_loop_thread_init(struct event_loop_thread *, int);

//主线程调用，初始化一个子线程，并且让子线程运行event_loop
struct event_loop *event_loop_thread_start(struct event_loop_thread*);

#endif