#ifndef __SIZE_H_
#define __SIZE_H_


#define   K_MAX_THREAD_CACHE_SIZE  (4 << 20) // 4 * 2^20 / 2^10 = 4K * 1024
#define   K_MIN_THREAD_CACHE_SIZE  K_MAX_SIZE * 2

//2169
#define   K_CLASS_ARRAY_SIZE ((K_MAX_SIZE + 127 + (120 << 7)) >> 7) + 1

#define   K_PAGE_SHIFT      13 
#define   K_PAGE_SIZE       1 << K_PAGE_SHIFT

#define   K_MAX_SIZE   256 * 1024 //256K
#define   K_MAX_SMALL_SIZE   1024

#define   K_ALIGNMENT_SIZE  8     //8b 准线

#define   K_NUM_CLASSES     86

#define   K_ADDR_BITS   (sizeof(void *) < 8 ? (8 * sizeof(void *)) : 48)


#define ASSERT(x)                           \
do {                                        \
     if (!(x))                              \
         debug1_msg("assertion failed");    \
} while (0)                                 


#endif//end __size_h_

