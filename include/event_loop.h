#ifndef __EVENT_LOOP
#define __EVENT_LOOP

#include <pthread.h>
#include "channel_map.h"
#include "event_dispatcher.h"
#include "require.h"
#include "timer.h"

extern const struct event_dispatcher poll_dispatcher;
extern const struct event_dispatcher epoll_dispatcher;

#define PENDING_ADD      1
#define PENDING_REMOVE   2
#define PENDING_UPDATE   3

//channe链表记录channel类型
struct channel_list {
    int                     type;
    struct channel*         channel;
    struct channel_list*    next;
};

#define Lock(m) pthread_mutex_lock(m)
#define Unlock(m) pthread_mutex_unlock(m)

struct event_loop {
    int quit;
    const struct event_dispatcher*  eventDispatcher;

    /*dispatcher数据*/
    void*                           event_dispatcher_data;
    /*channel 哈希*/
    struct channel_map*             channelMap;

    int is_handle_pending;

    struct channel_list*         pending_head;
    struct channel_list*         pending_tail;

    pthread_t                       owner_thread_id;
    pthread_mutex_t                 mutex;
    pthread_cond_t                  cond;
    /*主eventLoop线程与子eventLoop线程通讯*/
    int                             socketPair[2];
    char*                           thread_name;

    /*定时器相关 且只有mainLoop使用*/
    struct sort_timer_lst*          timerList;
    int                             timeout; //0未超时 1超时 
};


struct event_loop *event_loop_init();

struct event_loop *event_loop_init_with_name(char *);

int event_loop_run(struct event_loop *);

void event_loop_wakeup(struct event_loop*);

int event_loop_add_channel_event(struct event_loop *, struct channel*);

int event_loop_remove_channel_event(struct event_loop *, struct channel*);

int event_loop_update_channel_event(struct event_loop *, struct channel*);

int event_loop_handle_pending_channel(struct event_loop *eventLoop) ;

int event_loop_handle_pending_add(struct event_loop *, struct channel*);

int event_loop_handle_pending_del(struct event_loop *, struct channel*);

int event_loop_handle_pending_remove(struct event_loop *, struct channel*);

int event_loop_handle_pending_update(struct event_loop *, struct channel*);
// dispather派发完事件之后，调用该方法通知event_loop执行对应事件的相关callback方法
// res: EVENT_READ | EVENT_READ等
int channel_event_activate(struct event_loop *eventLoop, int fd, int res);
#endif