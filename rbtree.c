#include "rbtree.h"

/* 设置节点为黑色 */
static inline void rb_set_black(struct rb_node *rb)
{
    rb->__rb_parent_color |= RB_BLACK;  /* x | 1 = 1 */
}

/* 取得红色节点的父亲地址，就是red->__rb_parent_color 本身 */
static inline struct rb_node *rb_red_parent(struct rb_node *red)
{
    return (struct rb_node *)red->__rb_parent_color;
}

/* 旋转操作的辅助函数 */
//功能： 将new的父节点指针和颜色设置为old的父节点和颜色，将后将old的父节
//点设置为new，并将颜色设置为color，最后使得old的父节点指向正确的孩子，
//这个功能由 __rb_change_child完成
static inline void
__rb_rotate_set_parents(struct rb_node *old, struct rb_node *new,
            struct rb_root *root, int color)
{
    struct rb_node *parent = rb_parent(old);         /*取得父节点地址  */
    new->__rb_parent_color = old->__rb_parent_color; /*旧节点升级为父亲*/
    rb_set_parent_color(old, new, color);            /*更改旧节点的颜色*/
    __rb_change_child(old, new, parent, root);
}


                        /* 调整红黑树使之符合红黑树的特性 */
/*
 *
 *新插入的节点总是红色的，所以将RB_RED定义为0，新节点的指向父节点的指针的最后一位总是
 *为0，另外，新插入的节点的初始位置总是在最底层（不考虑NULL节点的叶子），调整位置时不
 * 断把他上移。1.如果插入的节点是根节点，就破坏了性质2,如果新插入节点的父节点是红色，
 * 就破坏了性质4,其他性质是不会破坏的。
 *
 *对于破坏性质的情况分析：
 * 正在处理的节点称为当前节点，待插入节点总是第一个当前节点，当前节点是红色的，并且当
 * 前节点的两个子树都是满足所有的性质
 *
 *1. 父节点是NULL，说明当前节点是根节点，只需要把他染成黑色即可
 *2. 父节点是黑色，性质2和性质4显然没有破坏，因为当前节点是红色的，所以性质5也没有破坏
 *3. 父节点是红色，并且祖父节点的另一个孩子（叔叔节点）是红色，显然破坏性质4
 *4. 父节点是红色，并且叔叔节点是黑色，当前节点是父节点的右孩子，用样破坏性质4
 *5. 父节点是红色，并且叔叔节点是黑色，当前节点是父节点的左孩子，同样破坏性质4
 *
 */

static inline void
__rb_insert(struct rb_node *node, struct rb_root *root,
        void (*augment_rotate)(struct rb_node *old, struct rb_node *new))
{                   /*新插入节点指针域为NIL, 并且着为红色*/
    
    struct rb_node *parent = rb_red_parent(node), *gparent, *tmp;

    while (true) {
        /*
         * Loop invariant: node is red
         *
         * If there is a black parent, wa are done. Otherwise, take some 
         * corrective action as we dont't want a red root or two 
         * consecutive red nodes.
         */
/*---------------------------在第一个while循环中进行矫正------------------*/
        if (!parent) {
            rb_set_parent_color(node, NULL, RB_BLACK); //根节点是黑色
            break;
        } else if (rb_is_black(parent))//只有当n节点的父亲是红色时才需要修正
            break;
/*-------------------------------------------------------------------------*/

//说明：        
/*  大写 : --black 
 *  小写 : --red
 *  g    : --gparent
 *  u    : --uncle
 *  p    : --parent
 *  n    : --node
 */
        gparent = rb_red_parent(parent); /*n节点的祖父节点g*/
        tmp = gparent->rb_right;         /* 祖父节点的右孩子，也是n节点的叔叔u */

        if (parent != tmp) {             /* parent == gparent->rb_left */

        /*大前提：  设 n节点的父节点p是祖父节点g的左孩子，且为红色 */
            
            if (tmp && rb_is_red(tmp)) { //*设 n节点的叔叔u也是红色
                /*
                 *     情况3：n 的叔叔是红色的.
                 *
                 *        G               g
                 *       / \             / \
                 *      p   u    ->     P   U 
                 *     /               /
                 *    n               n
                 *
                 *解决办法：n的父亲和叔叔都是红色的，根据性质4，n的父亲的
父亲必然是黑色的，那么可以将n的父亲和叔叔都变变为黑色（此时n和n的父亲不再是
连续的红色，但是该路径上黑色节点的个数增加了1），但是已祖父节点为根的路径上
黑节点数还是相同的，并且以父节点和以叔叔节点为根的子树都满足上述条件4,都是
红黑树。再把n的父亲的父亲变为红色（此时该路径上的黑色节点个数不变）。现在
我们可以把n的父亲的父亲当作新的n,因为其父亲仍然可能是红色的，于是又出现了
连续两个红色节点的问题。但是此时n的高度在树中移了两层，是一个稍微简单的子
问题，可以递归或者循环进行解决。
                 */
                        /* 同时更改父亲和叔叔为黑色 */
                rb_set_parent_color(tmp, gparent, RB_BLACK);
                rb_set_parent_color(parent, gparent, RB_BLACK);
                node = gparent; //将祖父节点设置为当前节点, 高度上移两层
                parent = rb_parent(node); //取得新的当前节点的父节点
                rb_set_parent_color(node, parent, RB_RED);//染为红色
                continue;           //上去继续循环
            }

            tmp = parent->rb_right;
            if (node == tmp) {
                /*情况4：n的叔叔u是黑色的，而且n是右孩子
                 *Case2: left rotate at parent.
                 *      G            G
                 *     / \          / \
                 *    p   U   -->  n   U
                 *     \          /
                 *     n         p
                 *
                 *解决：简单的将n变成n的父亲，然后对n做一次左旋,转化为情况5
                 *
                 *            p                 z
                 *           / \               / \ 
                 *          a   z    -->      p   c
                 *             / \           / \
                 *            b   c         a   b
                 */
                /*修正parent(p)与和node(n)的左儿子b之间的链接(左旋）*/
                parent->rb_right = tmp = node->rb_left;
                node->rb_left = parent;

                if (tmp)
                    rb_set_parent_color(tmp, parent, RB_BLACK);
                rb_set_parent_color(parent, node, RB_RED);
                augment_rotate(parent, node);                   //啥也没干
                parent = node;  //调整parent和node的指向,将当前节点设置为parent
                tmp = node->rb_right;//一切为了转化为情况5
            }

            /*情况5：n的叔叔u是黑色的，而且n是左孩子
             *Case5：right rotate at gparent
             *
             *          G                  P
             *         / \                / \
             *        p   U     -->      n   g
             *       /                        \
             *      n                          U  
             *解决办法：此时n和n的父亲都是红色的，而n的父亲的父亲肯定是黑色
的，我们把n的父亲变成黑色，n的父亲的父亲变成红色，这样对性质5不会有影响，然
后再对n的父亲的父亲做一次右旋操作，此时彻底解决了n上存在两个连续红色节点的
问题,修正完毕.
             */

            /*                   右旋操作                   */
            gparent->rb_left = tmp;  /* == parent->rb_right */
            parent->rb_right = gparent;
            if (tmp)
                rb_set_parent_color(tmp, gparent, RB_BLACK);

          /* 旋转操作的协作函数,修正曾祖父节点、祖父节点,父节点内容 */
            __rb_rotate_set_parents(gparent, parent, root , RB_RED);
            augment_rotate(gparent, parent); //什么都不干
            break;
        } else {

     /*大前提：  设 当前节点n的父亲自身是个右孩子，且为红色! */
            
            //处理遇上边类似.....
            tmp = gparent->rb_left;
            if (tmp && rb_is_red(tmp)) {
                 /* Case 3 - color flips */
                rb_set_parent_color(tmp, gparent, RB_BLACK);
                rb_set_parent_color(parent, gparent, RB_BLACK);
                node = gparent;
                parent = rb_parent(node);
                rb_set_parent_color(node, parent, RB_RED);
                continue;
            }

            tmp = parent->rb_left;
            if (node == tmp) {
                /* Case 4 - right rotate at parent */
                parent->rb_left = tmp = node->rb_right;
                node->rb_right = parent;
                if (tmp)
                    rb_set_parent_color(tmp, parent, RB_BLACK);
                rb_set_parent_color(parent, node, RB_RED);
                augment_rotate(parent, node);
                parent = node;
                tmp = node->rb_left;
            }

            /* Case 5 - left rotate at parent */
            gparent->rb_right = tmp;  /* == parent->rb_left */
            parent->rb_left = gparent;
            if (tmp)
                rb_set_parent_color(tmp, gparent, RB_BLACK);
            __rb_rotate_set_parents(gparent, parent, root, RB_RED);
            augment_rotate(gparent, parent);
            break;
        }
    }
}

static inline void
____rb_erase_color(struct rb_node *parent, struct rb_root *root,
        void (*augment_rotate)(struct rb_node *old, struct rb_node *new))
{
    struct rb_node *node = NULL, *sibling, *tmp1, *tmp2;

    while (true) {
        /*
         *Loop invariant:
         *-node is black (or NULL on first iteration)
         *-node is not the root (parent is not NULL)
         *-All leaf paths going through parent and node have a black node 
         * count that is 1 lower than other left paths.
         */
        sibling = parent->rb_right;
    /*首先，从它的左孩子来分析，第一次迭代时node是NULL节点....*/
        if (node != sibling) {  /* node == parent->rb_left */
            if (rb_is_red(sibling)) {//node的兄弟节点是红色的情况
                /*
                 * Case 1: left rotate at parent
                 *
                 *       P                S
                 *      / \              / \
                 *     N   s    -->     p   Sr
                 *        / \          / \
                 *       Sl  Sr       N   Sl
                 */
                //父节点一定是黑色，以父节点为根左旋
                parent->rb_right = tmp1 = sibling->rb_left;
                sibling->rb_left = parent;
                //将父节点与兄弟节点，颜色互换。
                rb_set_parent_color(tmp1, parent, RB_BLACK);
                __rb_rotate_set_parents(parent, sibling, root, RB_RED);
                augment_rotate(parent, sibling);
                sibling = tmp1;
            }//完成后，当前节点N未变，兄弟节点sibling变成tmp1
            tmp1 = sibling->rb_right;
    //如果兄弟节点的右孩子要么是黑节点，要么不存在（，因为兄弟
    //节点已经是黑色了，他不可能只有一个黑色的孩子，前面说过）
    //把兄弟节点染红，兄弟节点变红后经过它的路径的黑节点数就少1
    //和当前节点所在路径上黑节点数相同。如果父节点是红色，那么把
    //父节点染黑后，他的路径上黑节点数就增加1，满足性质5，退出，
    //如果父节点是黑色并且父节点不是根节点，那么经过父节点的路径
    //上的黑节点说比其它路径上少1，把父节点设为当前节点，继续循环.
            if (!tmp1 || rb_is_black(tmp1)) {
                tmp2 = sibling->rb_left;
                if (!tmp2 || rb_is_black(tmp2)) {
                    /*
                     *Case2: sibling color flip (p could be either color )
                     *
                     *        (p)                    (p)
                     *        / \                    / \
                     *       N   S        -->       N   s
                     *          / \                    / \
                     *         Sl  Sr                 Sl Sr
                     *
                     */
                    rb_set_parent_color(sibling, parent, RB_RED);//染红
                    if (rb_is_red(parent))
                        rb_set_black(parent);
                    else {
                        node = parent;
                        parent = rb_parent(node);
                        if (parent)
                            continue;
                    }
                    break;
                }
                /*
                 *Case3: right rotate at sibling
                 * (p could be either color here)
                 *
                 *          (p)             (p)
                 *          / \             / \
                 *         N   S     -->   N   Sl
                 *            / \               \
                 *           sl  Sr              s
                 *                                \
                 *                                 Sr 
                 */
    //如果兄弟节点的右孩子不存在或者黑色，兄弟节点的左孩子是红色
    //以兄弟节点为根右旋，父节点的颜色没有影响                         
                sibling->rb_left = tmp1 = tmp2->rb_right;
                tmp2->rb_right = sibling;
                parent->rb_right = tmp2;
                if (tmp1)
                    rb_set_parent_color(tmp1, sibling, RB_BLACK);
                augment_rotate(sibling, tmp2);
                tmp1 = sibling;
                sibling = tmp2;
            }
            /*
             * Case4 : left rotate at parent + color flips
             *
             *             (p)                (s)
             *             / \                / \
             *            N   S      -->     P   Sr
             *               / \            / \
             *             (sl) sr         N  (sl)
             *
             */
     //下面这段代码用于继续处理上面的结果，还可以处理兄弟节点的右孩子
     //是红色的情况，它做的是 左旋 + 染色.       
            parent->rb_right = tmp2 = sibling->rb_left;
            sibling->rb_left = parent;
            rb_set_parent_color(tmp1, sibling, RB_BLACK);
            if (tmp2)
                rb_set_parent(tmp2, parent);
            __rb_rotate_set_parents(parent, sibling, root, RB_BLACK);
            augment_rotate(parent, sibling);
      //经过当前节点的路径上的黑节点数会比其他路径少1，经过这样的调整后
      //经过当前节点的路径上的黑节点数增加了1，满足性质5，调整完毕.
            break;

/*当前节点是父节点右孩子的处理与它是左孩子的处理类似*/    
        } else {
            sibling = parent->rb_left;
            if (rb_is_red(sibling)) {
                /* Case1: right rotate at parent */
                parent->rb_left = tmp1 = sibling->rb_right;
                sibling->rb_right = parent;
                rb_set_parent_color(tmp1, parent, RB_BLACK);
                __rb_rotate_set_parents(parent, sibling, root, RB_RED);
                augment_rotate(parent, sibling);
                sibling = tmp1;
            }
            tmp1 = sibling->rb_left;
            if (!tmp1 || rb_is_black(tmp1)) {
                tmp2 = sibling->rb_right;
                if (!tmp2 || rb_is_black(tmp2)) {
                    /* Case 2: - sibling color flip */
                    rb_set_parent_color(sibling, parent, RB_RED);
                    if (rb_is_red(parent))
                        rb_set_black(parent);
                    else {
                        node = parent;
                            parent = rb_parent(node);
                        if (parent)
                            continue;
                    }
                    break;
                }
                /* Case3 : right rotate at sibling */
                sibling->rb_right = tmp1 = tmp2->rb_left;
                tmp2->rb_left = sibling;
                parent->rb_left = tmp2;
                if (tmp1)
                    rb_set_parent_color(tmp1, sibling, RB_BLACK);
                augment_rotate(sibling, tmp2);
                tmp1 = sibling;
                sibling = tmp2;
            }
            /* Case4: - left rotate  at parent + color flips */
            parent->rb_left = tmp2 = sibling->rb_right;
            sibling->rb_right = parent;
            rb_set_parent_color(tmp1, sibling, RB_BLACK);
            if (tmp2)
                rb_set_parent(tmp2, parent);
            __rb_rotate_set_parents(parent, sibling, root, RB_BLACK);
            augment_rotate(parent, sibling);
            break;
        }
    }
}

void __rb_erase_color(struct rb_node *parent, struct rb_root *root,
        void (*augment_rotate)(struct rb_node *old, struct rb_node *new))
{
    ____rb_erase_color(parent, root, augment_rotate);
}


/* 在普通版本中， 这三个函数什么都不做，回调函数主要用在增强版本中 */
static inline void dummy_propagate(struct rb_node *node, 
                                                  struct rb_node *stop) {}
static inline void dummy_copy(struct rb_node *old, struct rb_node *new) {}
static inline void dummy_rotate(struct rb_node *old, struct rb_node *new) {}

//封装3个回调函数.
static const struct rb_augment_callbacks dummy_callbacks = {
    dummy_propagate, dummy_copy, dummy_rotate
};

//执行插入操作，立即将任务转交给 __rb_insert
// node : 待插入的节点
// root : 树根
// dummy_rotate :什么都没做，普通版本！<主要用在增强版本中>
void rb_insert_color(struct rb_node *node, struct rb_root *root)
{
    __rb_insert(node, root, dummy_rotate);
}

void rb_erase(struct rb_node *node, struct rb_root *root)
{
    struct rb_node *rebalance;
    rebalance = __rb_erase_augmented(node, root, &dummy_callbacks);
    if (rebalance)
        ____rb_erase_color(rebalance, root, dummy_rotate);
}

//调整红黑树使之符合红黑树的特性
void __rb_insert_augmented(struct rb_node *node, struct rb_root *root,
        void (*augment_rotate)(struct rb_node *old, struct rb_node *new))
{
    __rb_insert(node, root, augment_rotate);
}

/*
 * This function returns the first node (int sort order) of the tree.
 * 获取最左边的节点，也就是最小值.
 */
struct rb_node *rb_first(const struct rb_root *root)
{
    struct rb_node *n;
    n = root->rb_node;

    if (!n)
        return NULL;
    while (n->rb_left)
        n = n->rb_left;
    return n;
}

 /* 获取最右边的节点，也就是最大值. */
struct rb_node *rb_last(const struct rb_root *root)
{
    struct rb_node *n;
    n = root->rb_node;
    if (!n)
        return NULL;
    while (n->rb_right)
        n = n->rb_right;
    return n;
}

/* 用于中序遍历， 获取当前节点的下一个节点 */
struct rb_node *rb_next(const struct rb_node *node)
{
    struct rb_node *parent;

    if (RB_EMPTY_NODE(node))//判断节点是否在红黑树中
        return NULL;
 /*If we have a right-hand child, go down and then left as for as we can*/
    //如果当前节点有右孩子，那么它的下一个节点一定在右子树的最左端，如果
    //没有，因为它左子树的节点都比它小，如果当前节点是父节点的左孩子，那么
    //父节点就是它的下一个节点；否则，需要顺着父节点方向向上，找到第一个是
    //父节点的左孩子的节点，他的父节点就是当前节点的下一个节点。
    //         P :当前节点           N：下一个节点                  
    //    P                   N                        N
    //     \                 / \                      /
    //     ( )             P   ( )                   ()  
    //     /                                           \
    //    N                                             ()
    //                                                    \
    //                                                     P
    if (node->rb_right) {
        node = node->rb_right;
        while (node->rb_left)
            node = node->rb_left;
        return (struct rb_node *)node;
    }

    while ((parent = rb_parent(node)) && node == parent->rb_right)
        node = parent;

    return parent;
}

/* 获取当前节点的前一个节点 */
struct rb_node *rb_prev(const struct rb_node *node)
{
    struct rb_node *parent;

    if (RB_EMPTY_NODE(node))
        return NULL;
/* If we have a left-hand child, go down and then right as far as we can.*/

    if (node->rb_left) {
        node = node->rb_left;
        while (node->rb_right)
            node = node->rb_right;
        return (struct rb_node *)node;
    }
/*   No left-hand children. Go up till we find an ancestor which is a 
 *   right-hand child of its parent.                                     */
    while ((parent = rb_parent(node)) && node == parent->rb_left)
        node = parent;

    return parent;
}

//节点的替换工作，仅仅需要修改周围节点的指针方向，并将原节点的指针和颜色拷贝给新结点
void rb_replace_node(struct rb_node *victim, struct rb_node *new,
                                                   struct rb_root *root)
{
    struct rb_node *parent = rb_parent(victim);
    /* Set the surrounding nodes to point to the replacement */
    __rb_change_child(victim, new, parent, root);
    if (victim->rb_left)
        rb_set_parent(victim->rb_left, new);
    if (victim->rb_right)
        rb_set_parent(victim->rb_right, new);

    /*Copy the pointers/colour from victim to the replacement  */
    *new = *victim;
}

//用于获取参数node为根的最左边的叶子节点
static struct rb_node *rb_left_deepest_node(const struct rb_node *node)
{
    for (;;) {
        if (node->rb_left)
            node = node->rb_left;
        else if (node->rb_right)
            node = node->rb_right;
        else
            return (struct rb_node *)node;
    }
}

//用于后序遍历中寻找当前节点的下一个节点
struct rb_node *rb_next_postorder(const struct rb_node *node)
{
    const struct rb_node *parent;
    if (!node)
        return NULL;
    parent = rb_parent(node);

    /* If we're sitting on node, we've already seen our children */
    if (parent && node == parent->rb_left && parent->rb_right) {
        /* If we are the parent's left node, go to the parent's right
         * node then all the way down to the left */
        return rb_left_deepest_node(parent->rb_right);
    } else
        /* Otherwise we are the parent's right node, and the parent
         * should be next */
        return (struct rb_node *)parent;
}


//用于获取后续遍历的第一个节点
struct rb_node *rb_first_postorder(const struct rb_root *root)
{
    if (!root->rb_node)
        return NULL;

    return rb_left_deepest_node(root->rb_node);
}

