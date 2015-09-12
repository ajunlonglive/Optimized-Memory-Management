#include "hemalloc.h"
#include "mutex.h"
#include "sbrk.h"

int    has_initalized = FALSE ;
malloc_mutex_t        m_mutex;
struct mm_centre_ctl  m_centre_ctl;

//红黑树插入操作
static void Insert(struct tree_node *node, struct rb_root *root)
{
    struct rb_node **new = &root->rb_node;
    struct rb_node *parent = NULL;
    long   Key = node->Address;

    while (*new) {
        parent = *new; //从根节点开始试探
        if (Key < rb_entry(parent, struct tree_node, rb)->Address)
            new = &parent->rb_left;
        else
            new = &parent->rb_right;
    }
    rb_link_node(&node->rb, parent, new);
    rb_insert_color(&node->rb, root);
}

//查找操作
static struct rb_root *find_root(void *freeAddr)
{
    //TODO...struct tree_node *
}

static struct rb_node *Search(struct rb_root *root, void *addr)
{
    struct rb_node *node = root->rb_node;
    while(node) {
        struct tree_node *data = rb_entry(node, struct tree_node, rb);
        long Key = data->Address;

        if ((long)addr < Key)
            node = node->rb_left;
        else if ((long)addr > Key)
            node = node->rb_right;
        else
            return node;
    }
    return NULL;
}

//从红黑树上解下节点
static inline void Erase(struct tree_node *node, struct rb_root *root)
{
    rb_erase(&node->rb, root);
}

void init_tree_node(struct rb_root *root, int Class, int classSize, 
        void **N, void **S, SpaceLevel L)
{
    struct tree_node *treeNode = NULL;
    int i;
    for (i = 0; i < L; i++) {
        treeNode = (struct tree_node *) (*N);
        treeNode->class_index = Class;  //分组下标
        treeNode->Address = (long)(*S);
        treeNode->bitMap = NULL;
        //加入空闲树中
        Insert(treeNode, root);
        
        *N += RB_TREE_NODE_SIZE;
        *S += classSize;
    }
}


bool he_thcache_init(int tid, void *ctl_addr)
{
    size_t size;
    void *nodeAddr = ctl_addr + THREAD_EXTRA_SIZE;
    void *spaceAddr = nodeAddr + (TOTLE_NODE_SIZE * RB_TREE_NODE_SIZE);
    struct thread_block *ctl = (struct thread_block *)ctl_addr;

    ctl->is_alive = YES;
    ctl->th_tid = tid;
    INIT_LIST_HEAD(&(ctl->add_memory));
    //分组初始化
    int i,j,k;
    int class_size = 0;
    
    for (i = 0; i < RB_TREE_CLASS_NUM; i++) { //57个分组
        ctl->c_tree.aligmentTree[i].rb_node = NULL;
        ctl->c_tree.freeTree[i].rb_node = NULL;
        class_size = class_to_size(i + 1);//获取分组对应的大小

        switch (class_size) {
            case 0 ... 128:
                init_tree_node(&(ctl->c_tree.freeTree[i]), i, class_size,
                            &nodeAddr ,&spaceAddr, level1);
                break;
            case 144 ... 256:
                init_tree_node(&(ctl->c_tree.freeTree[i]), i, class_size,
                            &nodeAddr ,&spaceAddr, level2);
                break;
            case 288 ... 512:
                init_tree_node(&(ctl->c_tree.freeTree[i]), i, class_size,
                            &nodeAddr ,&spaceAddr, level3);
                break;
            case 576 ... 1024:
                init_tree_node(&(ctl->c_tree.freeTree[i]), i, class_size,
                            &nodeAddr ,&spaceAddr, level4);
                break;
            case 1152 ... 4096:
                init_tree_node(&(ctl->c_tree.freeTree[i]), i, class_size,
                            &nodeAddr ,&spaceAddr, level5);
                break;
        }

    }
    list_add_tail(&(ctl->th_node), &(m_centre_ctl.th_cache.t_head));
}

bool he_init_memory(void)
{
    if (malloc_mutex_init(&m_mutex) == FALSE) {
        err_msg("mutex init fail.");
        return (NG);
    }
    malloc_mutex_lock(&m_mutex);
    m_centre_ctl.mm_last_addr = sbrk(0); //抓取最后一个有效地址
                                          //作为管理的起始地址
    m_centre_ctl.mm_start_addr = m_centre_ctl.mm_last_addr;
    has_initalized = TRUE;                //已经初始化完毕

    if ((void *)-1 == sbrk(DEFAULT_MMAP_THRESHOLD)) {
        err_msg("sbrk init fail.");
        return (NG);
    }                                     //重新定位边界
    m_centre_ctl.mm_last_addr += DEFAULT_MMAP_THRESHOLD;
    init_size_map();

    m_centre_ctl.th_cache.t_cnt = 1;            //为主进程预备
    m_centre_ctl.th_cache.t_free_cache = 0;
    INIT_LIST_HEAD(&(m_centre_ctl.th_cache.t_head));
    INIT_LIST_HEAD(&m_centre_ctl.big_list_head);

    
    malloc_mutex_unlock(&m_mutex);
 
    return (OK);
}



int main(int ac, char **av)
{
    bool is_ok;
    is_ok = he_init_memory();


    if (is_ok)
        printf ("It's OK!\n");
    else
        printf ("It's error!\n");

    printf("class:%d \t\t size: %d\n", size_to_class(56), class_to_size(size_to_class(56)));

    printf("%d\n", RB_TREE_NODE_SIZE);
    printf("%d\n", THREAD_EXTRA_SIZE);
    printf("%d\n", THREAD_BLOCK_SIZE);
    printf("%d\n", BIG_BLOCK_SIZE);
    printf("%d\n", sizeof(size_t));
    return 0;
}
