#ifndef __TIMER_H
#define __TIMER_H

#include <time.h>

typedef int(*timeout_call_back)(void *data);

#define TIMESLOT 5
#define TIMEOUT 3
/*处理非活跃连接*/
struct sort_timer_lst;

struct timer {
    timeout_call_back   timeoutCallBack;
    time_t              expire_time;
    void *              data;

    struct timer*       next;
    struct timer*       prev;

    struct sort_timer_lst *list;
};

/*升序的定时器列表 环形双向链表*/
struct sort_timer_lst {
    struct timer *head;
    size_t      size;
};


/*定时器相关*/
struct timer * generate_timer(timeout_call_back, void *);
void update_timer(struct timer*, int);

struct sort_timer_lst *create_sort_timer_lst();

/*遍历定时器列表*/
void tick(struct sort_timer_lst *);

void add_timer(struct sort_timer_lst *, struct timer *);

void del_timer(struct sort_timer_lst *, struct timer *);

/*当接收到客户端活跃的信息，重置定时器*/
void reset_timer(struct sort_timer_lst *, struct timer *);


#endif