#ifndef __MUTEX_H_
#define __MUTEX_H_

#include <pthread.h>


//extern bool isthreaded;
typedef struct malloc_mutex_s malloc_mutex_t;

#define MALLOC_MUTEX_INITALIZER {PTHREAD_MUTEX_INITIALIZER}

#define MALLOC_MUTEX_TYPE PTHREAD_MUTEX_DEFAULT

struct malloc_mutex_s {
    pthread_mutex_t lock;
};


int malloc_mutex_init(malloc_mutex_t *mutex);
void malloc_mutex_lock(malloc_mutex_t *mutex);
void malloc_mutex_unlock(malloc_mutex_t *mutex);

#endif//end mutex_h_
