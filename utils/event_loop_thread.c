#include "../include/event_loop_thread.h"
#include "../include/log.h"
#include <assert.h>
#include <pthread.h>

/*初始化event_loop并通知主线程*/
void *event_loop_thread_run(void *arg) {
    struct event_loop_thread *eventLoopThread = (struct eventLoopThread*)arg;

    Lock(&eventLoopThread->mutex);
    //初始化sub_event_loop 创建socketPair套接字
    eventLoopThread->eventLoop = event_loop_init_with_name(eventLoopThread->thread_name);

    ember_msgx("event loop thread init and signal, %s", eventLoopThread->thread_name);
    //通知主线程
    pthread_cond_signal(&eventLoopThread->cond);

    Unlock(&eventLoopThread->mutex);

    event_loop_run(eventLoopThread->eventLoop);
}

/*初始化event_thread_init*/
int event_loop_thread_init(struct event_loop_thread *eventLoopThread, int i) {
    pthread_cond_init(&eventLoopThread->cond,NULL);
    pthread_mutex_init(&eventLoopThread->mutex,NULL);

    eventLoopThread->eventLoop = NULL;
    eventLoopThread->thread_count = 0;
    eventLoopThread->tid = 0;

    char *buf = malloc(16);

    sprintf(buf, "Thread-%d",i + 1);
    eventLoopThread->thread_name = buf;
    
    return 0;
}

/*初始化子线程并让子线程，运行event_loop_run*/
struct event_loop *event_loop_thread_start(struct event_loop_thread *eventLoopThread) {
    pthread_create(&eventLoopThread->tid,NULL,event_loop_thread_run,eventLoopThread);
    /*这一步用于通知主线程*/
    assert(Lock(&eventLoopThread->mutex) == 0);

    while (!eventLoopThread->eventLoop) {
        assert(pthread_cond_wait(&eventLoopThread->cond,&eventLoopThread->mutex) == 0);
    }

    assert(Unlock(&eventLoopThread->mutex) == 0);

    ember_msgx("event loop thread started, %s",eventLoopThread->thread_name);

    return eventLoopThread->eventLoop;
}