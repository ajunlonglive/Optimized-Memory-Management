#ifndef __LIST_H_
#define __LIST_H_

#include "container_of.h"


#define LIST_HEAD_INIT(name) { &(name), &(name) }

/* 声明一个链表头 */
#define LIST_HEAD(name)  \
    struct list_head name = LIST_HEAD_INIT(name)

/* 声明一个链表头指针 */
#define LIST_HEAD_P(name) \
    struct list_head *name = (struct list_head *)malloc(sizeof(struct list_head))

struct list_head {
    struct list_head *next, *prev;
};

/* 初始化链表， 指向自己 */
#define INIT_LIST_HEAD(ptr) do {  \
    (ptr)->next = (ptr); (ptr)->prev = (ptr); \
}while(0)


static inline void __list_add(struct list_head *new,
                   struct list_head *prev,
                   struct list_head *next)
{
    next->prev = new;    
    new->next = next;    
    new->prev = prev;    
    prev->next = new;    
}

/*头插法*/
static inline void list_add_head(struct list_head *new, struct list_head *head)
{
    __list_add(new, head, head->next);
}

/*尾插法*/
static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
    __list_add(new, head->prev, head);
}

/*孤立一个节点*/
static inline void __list_del(struct list_head *prev, struct list_head *next)
{
    next->prev = prev;
    prev->next = next;
}

static inline void __list_del_entry(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);
}

static inline void list_del(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);

//保证不在链表中的节点项不可访问--
                  //对LIST_POSITION1和LIST_POSITION2的访问都将引起页故障
    entry->next = LIST_POISON1;
    entry->prev = LIST_POISON2;
}

static inline void list_del_init(struct list_head *entry)
{
    __list_del_entry(entry);
    INIT_LIST_HEAD(entry);  //置为空链
}

/* 用头指针的next是否指向自己来判断链表是否为空 */
static inline int list_empty(const struct list_head *head)
{
    return head->next == head;
}

#define list_for_each(pos, head) \
    for(pos = (head)->next; pos != (head); pos = pos->next)

#define list_del_for_each(pos, head) \
    for(pos = (head)->next; pos != (head); pos = (head)->next)  

#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)



/*---------------------------正向遍历----------------------------------*/
#define list_first_entry(ptr, type, member)  \
    list_entry((ptr)->next, type, member)

#define list_next_entry(pos, member) \
    list_entry((pos)->member.next, typeof(*(pos)), member)

/**
  * list_for_each_entry  -   iterate over list of given type
  * @pos:    the type * to use as a loop cursor.
  * @head:   the head for your list.
  * @member: the name of the list_head within the struct.
  */
#define list_for_each_entry(pos, head, member)      \
    for(pos = list_first_entry(head, typeof(*pos), member);  \
        &pos->member != (head);     \
        pos = list_next_entry(pos, member))
/*---------------------------------------------------------------------*/



/*--------------------------逆向遍历----------------------------------*/

#define list_last_entry(ptr, type, member) \
    list_entry((ptr)->prev, type, member)


#define list_prev_entry(pos, member)  \
    list_entry((pos)->member.prev, typeof(*(pos)), member)
/**
 *list_for_each_entry_reverse - iterate backwards over list of given type.
 * @pos:    the type * to use as a loop cursor.
 * @head:   the head for your list.
 * @member: the name of the list_head within the struct.
 */
#define list_for_each_entry_reverse(pos, head, member)     \
    for(pos = list_last_entry(head, typeof(*pos), member);    \
        &pos->member != (head);       \
        pos = list_prev_entry(pos, member))
/*---------------------------------------------------------------------*/

#define list_prepare_entry(pos, head, member) \
    ((pos) ? : list_entry(head, typeof(*pos), member))
        


/*--------------------------给定节点正向遍历---------------------------*/
#if 0
#define list_next_entry(pos, member) \
     list_entry((pos)->member.next, typeof(*(pos)), member)
#endif

#define list_for_each_entry_continue(pos, head, member)         \
     for (pos = list_next_entry(pos, member);            \
          &pos->member != (head);                    \
          pos = list_next_entry(pos, member))


/*--------------------------给定节点逆向遍历---------------------------*/
#if 0
#define list_prev_entry(pos, member) \
    list_entry((pos)->member.prev, typeof(*(pos)), member)
#endif

#define list_for_each_entry_continue_reverse(pos, head, member)     \
          for (pos = list_prev_entry(pos, member);            \
          &pos->member != (head);                    \
          pos = list_prev_entry(pos, member))


#endif //end __LIST_H_
