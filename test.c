#include <stdio.h>
#include "rbtree.h"

struct RB_tree {
    int  Key;
    char Alpha;
    struct rb_node rb;
};


static void insert(struct RB_tree *node, struct rb_root *root)
{
    struct rb_node **new = &root->rb_node;
    struct rb_node  *parent = NULL;
    int key = node->Key;

    while (*new) {
        parent = *new; //从根节点开始试探
        if (key < rb_entry(parent, struct RB_tree, rb)->Key)
            new = &parent->rb_left;
        else 
            new = &parent->rb_right;
    }
    rb_link_node(&node->rb, parent, new);
    rb_insert_color(&node->rb, root);
}


static inline void erase(struct RB_tree *node, struct rb_root *root)
{
    rb_erase(&node->rb, root);
}

static int is_red(struct rb_node *rb)
{
    return !(rb->__rb_parent_color & 1);
}

void display(struct rb_node *rb)
{
    struct RB_tree *result;
    if (rb) {
        result = rb_entry(rb, struct RB_tree, rb);
        printf("KEY: %d   COL: %s \n", result->Key, is_red(rb) ? "red" : "black");
        display(rb->rb_left);
        display(rb->rb_right);
    }
}

void display2(struct rb_node *rb)
{
    struct RB_tree *result;
    if (rb) {
        display2(rb->rb_left);
        result = rb_entry(rb, struct RB_tree, rb);
        printf("KEY: %d   COL: %s \n", result->Key, is_red(rb) ? "red" : "black");
        display2(rb->rb_right);
    }
}
int main(int ac, char **av)
{
    struct rb_root root = RB_ROOT;
    struct RB_tree NODE[10];
    int i;


    int a[] = {2, 14, 7, 15, 1, 5, 11, 8, 4, 100};

    for (i = 0; i < 10; i++) {
        NODE[i].Key = a[i];
        insert(NODE+i, &root);
    }
    printf("插入完毕！\n");
    //display(root.rb_node);
    display2(root.rb_node);

    printf("输出完毕！\n");
    return 0;
}
