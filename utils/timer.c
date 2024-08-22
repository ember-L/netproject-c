#include "../include/timer.h"
#include "../include/log.h"
#include <stdlib.h>


/*双向链表辅助函数*/
void tail_insert(struct timer * head,struct timer *node);
void insert(struct timer * position,struct timer *node);
void remove(struct timer *);


struct timer* generate_timer (timeout_call_back timeoutCallBack, void * data) {
    struct timer *timer = malloc(sizeof(struct timer));

    time_t cur_time = time(NULL);
    timer->expire_time = cur_time + TIMEOUT * TIMESLOT;

    timer->timeoutCallBack = timeoutCallBack;
    // 回调函数所用参数
    if(data)    
        timer->data = data;

    return timer;
}

void update_timer(struct timer * timer,int timerout) {
    timer->expire_time = time(NULL) + timerout * TIMESLOT;
}


struct sort_timer_lst *create_sort_timer_lst (){
    struct sort_timer_lst * list = malloc(sizeof(struct sort_timer_lst));
    list->head = malloc(sizeof(struct timer));
    
    list->head->prev = list->head;
    list->head->next = list->head;

    list->size = 0;

    return list;
}


/*添加新定时器采用尾插方式*/
void add_timer(struct sort_timer_lst * list, struct timer * new_timer) {
    struct timer *head = list->head;
    struct timer *tmp = head->next;
    new_timer->list = list;

    while(tmp != head) {
        if(tmp->expire_time > new_timer->expire_time){
            insert(tmp, new_timer);
            break;
        }
        tmp = tmp->next;
    }
    /*尾插*/
    if(tmp == head)
        tail_insert(head, new_timer);

    list->size++;
}

void del_timer(struct sort_timer_lst * list, struct timer * timer) {
    struct timer *tmp = list->head;

    while(tmp != list->head) {
        if(tmp == timer) {
            remove(timer);
            free(timer);
            break;
        }
        tmp = tmp->next;
    }

    list->size--;
}

void reset_timer(struct sort_timer_lst * list, struct timer * timer) {
    struct timer *head = list->head;
    if (timer == head->prev)
        return;
        ember_debugx("reset timer");
    /*将定时器移动到链表末端*/
    remove(timer);

    /*尾插 移动到链表尾部*/
    tail_insert(head, timer);
}

void tick(struct sort_timer_lst * list) {
    ember_msgx("tick...");
    struct timer *head = list->head;
    struct timer *tmp = head->next;
    time_t cur_time  = time(NULL);
    while(head != tmp) {
        if(cur_time < tmp->expire_time) 
            break;
        tmp->timeoutCallBack(tmp->data);

        /*删除链表中的定时器*/
        remove(tmp);

        list->size--;

        tmp = tmp->next;
    }
}

void tail_insert(struct timer *head, struct timer * node) {
    head->prev->next = node;
    node->prev = head->prev;
    head->prev = node;
    node->next = head;
}

void insert(struct timer * position,struct timer *node) {
    position->prev->next = node;
    node->prev = position->prev;
    node->next = position;
    position->prev = node;
}

void remove(struct timer * node) {
    node->next->prev = node->prev;
    node->prev->next = node->next;
}
