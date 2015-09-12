#ifndef __CONTAINER_OF_H_
#define __CONTAINER_OF_H_

                  /* 访问某个节点指向的数据项 */

#if defined(container_of)
    #undef  container_of
    #define container_of(ptr, type, member) ({          \
            const typeof( ((type *)0)->member ) *__mptr = (ptr);  \
            (type *)( (char *)__mptr - offsetof(type,member) );})
#else
    #define container_of(ptr, type, member) ({          \
            const typeof( ((type *)0)->member ) *__mptr = (ptr);  \
            (type *)( (char *)__mptr -offsetof(type,member) );})
#endif

#if defined(offsetof)
    #undef  offsetof                              /*常量结构体指针 0*/
    #define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#else
    #define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

#undef NULL
#if defined(__cplusplus)
    #define NULL 0
#else
    #define NULL ((void *)0)
#endif


#define POISON_POINTER_DELTA   0
/* 引发页面故障 */
#define LIST_POISON1 ((void *) 0x00100100 + POISON_POINTER_DELTA)
#define LIST_POISON2 ((void *) 0x00200200 + POISON_POINTER_DELTA) 

#endif //__container_of
