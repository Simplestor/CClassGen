#ifndef __LOCK_H__
#define __LOCK_H__

#include <pthread.h>

typedef struct{
    pthread_mutex_t lock;
}lock_t;

extern void lock_init(lock_t *);
extern void lock(lock_t *);
extern int try_lock(lock_t *);
extern void unlock(lock_t *);
extern void lock_destroy(lock_t *);

#endif //__LOCK_H__


