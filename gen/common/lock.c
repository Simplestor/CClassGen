#include "lock.h"

void lock_init(lock_t *lock)
{
    if(lock){
        pthread_mutex_init(&lock->lock, NULL);
    }
}

void lock(lock_t *lock)
{
    if(lock){
        pthread_mutex_lock(&lock->lock);
    }
}

int try_lock(lock_t *lock)
{
    if(lock){
        return pthread_mutex_trylock(&lock->lock);
    }

    return -1;
}

void unlock(lock_t *lock)
{
    if(lock){
        pthread_mutex_unlock(&lock->lock);
    }
}

void lock_destroy(lock_t *lock)
{
    if(lock){
        pthread_mutex_destroy(&lock->lock);
    }
}


