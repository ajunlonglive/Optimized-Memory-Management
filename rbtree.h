#ifndef	_LINUX_RBTREE_H
#define	_LINUX_RBTREE_H

#define true    1
#define false   0


#if defined(container_of)
  #undef container_of
  #define container_of(ptr, type, member) ({			\
        const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
        (type *)( (char *)__mptr - offsetof(type,member) );})
#else
  #define container_of(ptr, type, member) ({			\
        const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
        (type *)( (char *)__mptr - offsetof(type,member) );})
#endif

#if defined(offsetof)
  #undef offsetof
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

/*在Linux系统，32位对齐策略是short地址必须是2的倍数，即地址最后一位必为0
 *同理int 和 long 则要求地址为4的倍数，所以低两位必为0，如果在64位系统上
 *long 的地址必须是8的倍数，所以低三位必为0，因此将颜色比特位存放在父节点
 *最低一位上，方案可行，节约内存！
 */
struct rb_node {
    unsigned long __rb_parent_color;        /*最低一位是颜色位*/
    struct rb_node *rb_right;
    struct rb_node *rb_left;
} __attribute__((aligned(sizeof(long))));  //强调这里必须按long进行对齐.

struct rb_root {
    struct rb_node *rb_node;
};

/* 取得父节点的真实地址 */                                            
#define rb_parent(r) ((struct rb_node *)((r)->__rb_parent_color & ~3))
                                            /* 低两位清0        */
#define RB_ROOT (struct rb_root) {NULL, }   /* 初始化根节点指针 */
#define rb_entry(ptr, type, member) container_of(ptr, type, member)
                                            /* 取得数据地址     */
#define RB_EMPTY_ROOT(root) ((root)->rb_node == NULL) 
                                            /* 判断树空         */


/*'empty' nodes are nodes that are known not to be inserted in an rbtree */
                                  /* 判断节点是否为空,父亲是否等于自身 */
#define RB_EMPTY_NODE(node) \
    ((node)->__rb_parent_color == (unsigned long)(node))
                                  /* 设置节点为空，父亲等于自身        */ 
#define RB_CLEAR_NODE(node) \
    ((node)->__rb_parent_color = (unsigned long)(node))

extern void rb_insert_color(struct rb_node *, struct rb_root *);
extern void rb_erase(struct rb_node *, struct rb_root *);

/* Find logical next and previous nodes int a tree */
extern struct rb_node *rb_next(const struct  rb_node *);
extern struct rb_node *rb_prev(const struct  rb_node *);
extern struct rb_node *rb_first(const struct rb_root *);
extern struct rb_node *rb_last(const struct  rb_root *);

/* Postorder(后根序) iteration - always visite the parent after its child*/
extern struct rb_node *rb_first_postorder(const struct rb_root *);
extern struct rb_node *rb_next_postorder(const struct rb_node *);

/* Fast replacement of a signal node without remove/rebalance/add/ */
extern void rb_replace_node(struct rb_node *victim, struct rb_node *new,
                struct rb_root *root);


//将节点链接到红黑树中，node:待插入的节点 parent:待插入节点的父节点
//rb_link:指向的是父节点的左孩子或者右孩子，也就是在while循环中找到的位置
static inline void rb_link_node(struct rb_node *node, 
                           struct rb_node *parent, struct rb_node **rb_link)
{
    node->__rb_parent_color = (unsigned long)parent; //设置父节点
    node->rb_left = node->rb_right = NULL;           //初始化左右子树

    *rb_link = node; //指向新节点
}

#define rb_entry_safe(ptr, type, member) \
    ({ typeof(ptr) ____ptr = (ptr);      \
       ____ptr ? rb_entry(____ptr, type, member) : NULL; \
    })

#define rbtree_postorder_for_each_entry_safe(pos, n, root, field) \
    for (pos = rb_entry_safe(rb_first_postorder(root), typeof(*pos), field);\
    pos && ({ n = rb_entry_safe(rb_next_postorder(&pos->field), \
        typeof(*pos), field); 1; });      \
        pos = n)

/*插入操作需要修改节点信息，可以使用三个回调函数实现：
 *propagate 传播回调函数：更新node到它的祖先stop节点之间每一个节点的附加信息
 *copy      拷贝回调函数：将new节点的附加信息设置成old的附加信息
 *rotate    旋转回调函数：将new节点的附加信息设置成old的附加信息，
 *                        并重新计算old的附加信息
 */
struct rb_augment_callbacks {
    void (*propagate)(struct rb_node *node, struct rb_node *stop);
    void (*copy)(struct rb_node *old, struct rb_node *new);
    void (*rotate)(struct rb_node *old, struct rb_node *new);
};

extern void __rb_insert_augmented(struct rb_node *node, struct rb_root *root,
    void (*augment_rotate)(struct rb_node *old, struct rb_node *new));

static inline void
rb_insert_augmented(struct rb_node *node, struct rb_root *root,
            const struct rb_augment_callbacks *augment)
{
    __rb_insert_augmented(node, root, augment->rotate);
}

#define RB_DECLARE_CALLBACKS(rbstatic, rbname, rbstruct, rbfield, \
                  rbtype, rbaugmented, rbcompute)                 \
static inline void      \
rbname ## _propagate(struct rb_node *rb, struct rb_node *stop)\
{                                                             \
    while (rb != stop) {                                      \
        rbstruct *node = rb_entry(rb, abstruct, rbfield);     \
        rbtype augmented = rbcompute(node);                   \
        if (node->rbaugmented == augmented)                   \
            break;                                            \
        node->rbaugmented = augmented;                        \
        rb = rb_parent(&root->rbfield);                       \
    }                                                         \
}                                                             \
static inline void        \
rbname ## _copy(struct rb_node* rb_old, struct rb_node *rb_new) \
{                                                               \
    rbstruct *old = rb_entry(rb_old, rbstruct, rbfield);        \
    rbstruct *new = rb_entry(rb_new, rbstruct, rbfield);        \
    new->rbaugmented = old->rbaugmented;                        \
}                                                               \
static void               \
rbname ## _rotate(struct rb_node *rb_old, struct rb_node *rb_new)   \
{                                                                   \
    rbstruct *old = rb_entry(rb_old, rbstruct, rbfield);            \
    rbstruct *new = rb_entry(rb_new, rbstruct, rbfield);            \
    new->rbaugmented = old->rbaugmented;                            \
    old->rbaugmented = rbcompute(old);                              \
}                                                                   \
rbstatic const struct rb_augment_callbacks rbname = {          \
    rbname ## _propagate, rbname ## _copy, rbname ## _rotate   \
};

#define RB_RED     0
#define RB_BLACK   1

#define __rb_parent(pc)  ((struct rb_node *)(pc & ~3))  //低2位还原为0，得到父节点

#define __rb_color(pc)      ((pc) & 1)                  //取最后一位比特值
#define __rb_is_black(pc)   __rb_color(pc)              //最后一位为1 ?
#define __rb_is_red(pc)     (!__rb_color(pc))           //最后一位为0 ?

#define rb_color(rb)      __rb_color((rb)->__rb_parent_color)
#define rb_is_red(rb)     __rb_is_red((rb)->__rb_parent_color)
#define rb_is_black(rb)   __rb_is_black((rb)->__rb_parent_color)

/* 设置父节点的值 rb 节点本身  p 父节点真实地址 __rb_parent_color最终结果*/
static inline void rb_set_parent(struct rb_node *rb, struct rb_node *p)
{
    rb->__rb_parent_color = rb_color(rb) | (unsigned long)p;
}

/* 设置节点颜色域, rb 节点本身 p父节点真实值 color 颜色值 */
static inline void rb_set_parent_color(struct rb_node *rb,
            struct rb_node *p, int color)
{
    rb->__rb_parent_color = (unsigned long)p | color;
}

/* 让new 取代 old 在parent中的位置 */
static inline void 
__rb_change_child(struct rb_node *old, struct rb_node *new,
        struct rb_node *parent, struct rb_root *root)
{
    if (parent) {                       /* parent 不是树根  */
        if (parent->rb_left == old)     /* 如果原来是左孩子， 将新节点置左*/
            parent->rb_left = new;
        else                            /* 如果原来是右孩子， 将新节点置右*/
            parent->rb_right = new;
    } else                              /* parent 是根节点 */
        root->rb_node = new;
}

extern void __rb_erase_color(struct rb_node *parent, struct rb_root *root,
       void (*augment_rotate)(struct rb_node *old, struct rb_node *new));


/*删除节点，并且返回需要调整的节点（父节点）*/
static inline struct rb_node *
   __rb_erase_augmented(struct rb_node *node, struct rb_root *root,
              const struct rb_augment_callbacks *augment)
{
    struct rb_node *child = node->rb_right, *tmp = node->rb_left;
    struct rb_node *parent, *rebalance;
    unsigned long pc;

    if (!tmp) {//如果没有左孩子
        /*
         * Case 1: node to erase has no more than 1 child (easy!)
         */
    /*----------------------将待删除节点移除红黑树------------------------*/
        pc = node->__rb_parent_color;
        parent = __rb_parent(pc);
        __rb_change_child(node, child, parent, root);
    /*--------------------------------------------------------------------*/
        if (child) {//待删除节有且只有右孩子，则此节点必为黑色，孩子为红色
            child->__rb_parent_color = pc;//调整指向，并染黑.
            rebalance = NULL;
        } else     //待删除节点是红色，删除不需要调整！如果为黑色，破坏情况5
            rebalance = __rb_is_black(pc) ? parent : NULL;//从父节点重排
        tmp = parent;
    } else if (!child) {//右且只有左孩子，此节点必为黑色，孩子为红色
        /* Still case 1, but this time the child is node->rb_left */
        tmp->__rb_parent_color = pc = node->__rb_parent_color;
        parent = __rb_parent(pc);
        __rb_change_child(node, tmp, parent, root);
        rebalance = NULL;
        tmp = parent;
    } else {/*如果有两个孩子*/
/*解决办法： 选择它的右子树中最小者作为继承者，这个继承者将被放到它的
 *           位置并且继承它的颜色，这样可以保证只有经过它的右子树的路
 *           径上的黑节点数暂时改变，并且不会违背性质4，所以先找到继
 *           承者
 */        
        struct rb_node *successor = child, *child2;/*child = node->rb_right*/
        tmp = child->rb_left;
        if (!tmp) {//child没有左孩子,则child就是要找的继承者
            /*
             * Case 2: node's successor is its right child
             *
             *          (n)                 (s)
             *          / \                 / \
             *        (x) (s)    ->       (x) (c)
             *              \
             *              (c)
             */
            parent = successor;
            child = successor->rb_right;
            augment->copy(node, successor);
        } else {//否则顺着左边一直找到最小值，然后调整指针指向使结构不被破坏
            /*
             * Case 3: node's successor is leftmost under node's right 
             * child subtree
             *
             *          (n)                  (s)
             *          / \                  / \
             *        (x) (y)      ->      (x) (y)
             *            /                    /
             *          (p)                  (p)
             *          /                    /
             *        (s)                  (c)
             *          \
             *          (c)
             */
            do {
                parent = successor;
                successor = tmp;
                tmp = tmp->rb_left;
            } while (tmp);
            parent->rb_left = child2 = successor->rb_right;//继承者在原树中的右孩子
            successor->rb_right = child;
            rb_set_parent(child, successor);
            augment->copy(node, successor);
            augment->propagate(parent, successor);
        }//end else [case 3]
    /* 待删除节点的左子树将成为继承者的左子树，父节点将成为继承者的父节点*/
        successor->rb_left = tmp = node->rb_left;
        rb_set_parent(tmp, successor);

        pc = node->__rb_parent_color;
        tmp = __rb_parent(pc);
        //继承者继承待删除节点的颜色和位置，它被从原位置移出
        __rb_change_child(node, successor, tmp, root);
        if (child2) {//类似于if(child)之后的操作
            successor->__rb_parent_color = pc;
            rb_set_parent_color(child, parent, RB_BLACK);
            rebalance = NULL;
        } else {
            unsigned long pc2 = successor->__rb_parent_color;
            successor->__rb_parent_color = pc;
            rebalance = __rb_is_black(pc2) ? parent : NULL;
        }
        tmp = successor;
    }//end else [case 2]

    augment->propagate(tmp, NULL);
/*
*调整之后效果：可以想像成删除了一个黑色的叶子节点，含这个节点的路径上
*的黑节点数比起它路径少1，返回给____rb_erase_color的第一个参数是这个
*叶子节点的父节点.
*/
    return rebalance;
}

static inline void
rb_erase_augmented(struct rb_node *node, struct rb_root *root,
           const struct rb_augment_callbacks *augment)
{
    struct rb_node *rebalance = __rb_erase_augmented(node, root, augment);
    if (rebalance)
        __rb_erase_color(rebalance, root, augment->rotate);
}

#endif	/* _LINUX_RBTREE_H */
