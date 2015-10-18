#ifndef __SBRK_H_
#define __SBRK_H_

#include "rbtree.h"
#include "list.h"
#include "size.h"

#define PAGE_SIZE                   4096              //4k
#define DEFAULT_MMAP_THRESHOLD      20*1024*1024      //20M
#define PAGE_STEP                   4096              //4K

#define MIN_THREAD_SIZE             2*1024*1024        //2M

#define RB_TREE_CLASS_NUM           K_NUM_CLASSES-1   //除去size = 0;

#define TOTLE_NODE_SIZE             4448 //初始化红黑树节点总量174M

#define S_PROCESS         1                  //主进程
#define S_THREAD          0                  //线程

#define     YES           1                 
#define     NO            0

typedef enum {
    level1 = 256, 
    level2 = 128, 
    level3 = 64, 
    level4 = 32, 
    level5 = 16
}SpaceLevel;

/*------------------------红黑树节点------------------------------*/
struct tree_node {               //size = 48
    int  Position;
    int  class_index;         //所属分组
    long    Address;             //keys
    long    *bitMap;
    struct  rb_node rb;
};
#define RB_TREE_NODE_SIZE   sizeof(struct tree_node)


/*-----------------------线程堆维护信息---------------------------*/

struct class_rbtree {   //已分配红黑树、未分配红黑树
    struct rb_root freeTree[RB_TREE_CLASS_NUM];
    struct rb_root aligmentTree[RB_TREE_CLASS_NUM];
};

struct thread_extra_class {  //size = 32;
    bool is_using;                        //是否在使用中
    bool is_used_class;                   //是否应用于分组容器
    int  size_byte;                       //位图位数
    long    bitMap;                       //位图空间
    struct list_head extra_node;          //节点
};
#define THREAD_EXTRA_SIZE   sizeof(struct thread_extra_class)

//线程堆管理信息
struct thread_block {         //size:  952
    bool      is_alive;                        //线程状态
    int       th_tid;                          //线程ID
    struct    class_rbtree c_tree;             //56个容器分组
    struct    list_head add_memory; //额外增加的空间链表，回收的基本单位
    struct    list_head th_node;               //线程堆链表节点
};
#define THREAD_BLOCK_SIZE  sizeof(struct thread_block)


/*-------------------------中央维护信息-----------------------------*/

//中央维护的线程堆链表
struct thread_cache_ctl
{
    int t_cnt;
    int t_free_cache;
    struct list_head t_head; //线程堆链表头
};

//大内存块节点信息
struct big_block {           //size = 32
    bool         is_using;
    size_t       b_size;
    struct       list_head block_node;    //大块内存
};
#define BIG_BLOCK_SIZE  sizeof(struct big_block)


//中央控制块信息
struct mm_centre_ctl {
    unsigned long mm_size;
    struct thread_cache_ctl th_cache;
    struct list_head big_list_head;
    void *mm_border;
    void *mm_start_addr;
    void *mm_last_addr;
};

#endif //end __sbrk_h_
