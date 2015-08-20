#include "hemalloc.h"
#include "size.h"

unsigned char class_array_[K_CLASS_ARRAY_SIZE]; 
size_t class_to_size_[K_NUM_CLASSES];           
size_t class_to_pages_[K_NUM_CLASSES];


//计算 class_array_ 的索引值
static inline int class_index(int s)
{
    bool is_big      = (s > K_MAX_SMALL_SIZE);

    int add_amount   = is_big ? (127 + (120 << 7)) : 7;

    int shift_amount = is_big ? 7 : 3;

    return (s + add_amount) >> shift_amount;
}


inline int size_class(int size)
{
    return class_array_[class_index(size)];
}

inline size_t byte_size_for_class(size_t n)
{
    return class_to_size_[n];
}

inline size_t class_to_size(size_t n)
{
    return class_to_size_[n];
}

inline size_t class_to_pages(size_t n)
{
    return class_to_pages_[n];
}

//返回size值的最高位1的位置
static inline int LgFloor(size_t n)
{
    int log = 0;
    int i;

    for (i = 4; i >= 0; --i) {
        int shift = (1 << i);
        size_t x = n >> shift;

        if (x != 0) {
            n = x;
            log += shift;
        }
    }
    ASSERT(n == 1);
    
    return (log);
}

int alignment_for_size(size_t size) {
    int alignment = K_ALIGNMENT_SIZE;

    if (size > K_MAX_SIZE) {       // [256*1024, ...]
        alignment = K_PAGE_SIZE;
    } else if (size >= 128) {      // [128, 256*1024]
        alignment = (1 << LgFloor(size)) / 8;
    } else if (size >= 16)  {      //  [0, 16]
        alignment = 16;
    }

    if (alignment > K_PAGE_SIZE) {
        alignment = K_PAGE_SIZE;
    }

    return alignment;
}

int num_move_size(size_t size)
{
    int num;

    if (size == 0)
        return 0;
    num = 64 * 1024 / size;

    if (num < 2)
        num = 2;

    if (num > 32)
        num = 32;
    
    return num;
}

void init_size_map()
{
    int sc = 1;
    int alignment = K_ALIGNMENT_SIZE;
    size_t size;

    for (size = K_ALIGNMENT_SIZE; size <= K_MAX_SIZE; size += alignment) {
        alignment = alignment_for_size(size);

        int blocks_to_move = num_move_size(size) / 4;
        size_t psize = 0;
        do {
            psize += K_PAGE_SIZE;

            while ((psize % size) > (psize >> 3)) {
                psize += K_PAGE_SIZE;
            }
        } while ((psize / size) < (blocks_to_move));
        const size_t my_pages = psize >> K_PAGE_SHIFT;

        if (sc > 1 && my_pages == class_to_pages_[sc-1]) {
            const size_t my_objects = (my_pages << K_PAGE_SHIFT) / size;
            const size_t prev_objects = (class_to_pages_[sc-1] << K_PAGE_SHIFT) / class_to_size_[sc-1];
            if (my_objects == prev_objects) {
                class_to_size_[sc-1] = size;
                continue;
            }
        }
        class_to_pages_[sc] = my_pages;
        class_to_size_[sc] = size;
        sc++;
    }

    int next_size = 0;
    int c;

    for (c = 1; c < K_NUM_CLASSES; c++) {
        const int max_size_in_class = class_to_size_[c];
        int s;
        for (s = next_size; s <= max_size_in_class; s += K_ALIGNMENT_SIZE) {
            class_array_[class_index(s)] = c;
        }
        next_size = max_size_in_class + K_ALIGNMENT_SIZE;
    }

}

int main(int ac, char **av)
{
    int i = 0;
    size_t size;
    init_size_map();

    for (i = 0; i < 256*1024; i++)
        printf("index[%d] = %d\n", i, class_index(i));
printf("---\n\n\n");

    for (i = 0; i < K_CLASS_ARRAY_SIZE; i++)
        printf("class_arry[%d]   [%d]:\n", i, class_array_[i]);
printf("---\n\n\n");

    for (i = 0; i < K_NUM_CLASSES; i++)
        printf("class_to_size[%d] =  [%d]:\n", i, class_to_size_[i]);
printf("---\n\n\n");

    for (i = 0; i < K_NUM_CLASSES; i++)
        printf("class_to_pages_[%d] =  [%d]:\n", i, class_to_pages_[i]);

    return 0;
}
