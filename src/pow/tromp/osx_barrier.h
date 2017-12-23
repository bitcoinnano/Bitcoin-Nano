#ifndef POW_TROMP_OSX_BARRIER_H
#define POW_TROMP_OSX_BARRIER_H

#ifdef __APPLE__

#ifndef PTHREAD_BARRIER_H_
#define PTHREAD_BARRIER_H_

#include <pthread.h>
#include <errno.h>

typedef int pthread_barrierattr_t;
#define PTHREAD_BARRIER_SERIAL_THREAD 1

typedef struct
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int count;
    int tripCount;
} pthread_barrier_t;


int pthread_barrier_init(pthread_barrier_t *barrier, const pthread_barrierattr_t *attr, unsigned int count);

int pthread_barrier_destroy(pthread_barrier_t *barrier);

int pthread_barrier_wait(pthread_barrier_t *barrier);

#endif // PTHREAD_BARRIER_H_
#endif // __APPLE__

#endif // POW_TROMP_OSX_BARRIER_H
