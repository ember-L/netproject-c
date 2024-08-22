#include "../include/thread_pool.h"
#include <assert.h>
#include <stdlib.h>

struct thread_pool *thread_pool_new(struct event_loop *mainLoop, int nthread) {
    struct thread_pool *threadPoll = malloc(sizeof(struct thread_pool));
    threadPoll->mainLoop = mainLoop;
    threadPoll->position = 0;
    threadPoll->nthread = nthread;
    threadPoll->started = 0;
    threadPoll->eventLoopThreads = NULL;

    return threadPoll;
}


void thread_pool_start(struct thread_pool *threadPool) {
    assert(!threadPool->started);
    //必须是主线程进行控制
    assert(threadPool->mainLoop->owner_thread_id == pthread_self());

    threadPool->started = 1;

    if(threadPool->nthread <= 0)
        return;

    threadPool->eventLoopThreads = malloc(threadPool->nthread * sizeof(struct event_loop_thread));

    for (int i  = 0;i < threadPool->nthread ; i++) {
        event_loop_thread_init(&threadPool->eventLoopThreads[i],i);
        event_loop_thread_start(&threadPool->eventLoopThreads[i]);
    }
}


struct event_loop *thread_pool_get_loop(struct thread_pool *threadPool) {
    assert(threadPool->started);
    //必须是主线程进行控制
    assert(threadPool->mainLoop->owner_thread_id == pthread_self());

    struct event_loop *selected = threadPool->mainLoop;

    //从主线程挑选一个线程
    if(threadPool->nthread > 0) {
        selected = threadPool->eventLoopThreads[threadPool->position].eventLoop;
        if(++threadPool->position >= threadPool->nthread) {
            threadPool->position = 0;
        }
    }

    return selected;
}
