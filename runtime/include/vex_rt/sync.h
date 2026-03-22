#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include <pthread.h>

typedef struct { pthread_mutex_t inner; int poisoned; } VexMutex;
typedef struct { pthread_rwlock_t inner; int poisoned; } VexRwLock;
typedef struct { pthread_mutex_t mutex; pthread_cond_t cond; uint64_t count; } VexSemaphore;
typedef struct { pthread_barrier_t inner; } VexBarrier;
typedef struct { pthread_once_t inner; } VexOnce;

#define VEX_ONCE_INIT { PTHREAD_ONCE_INIT }

void vex_mutex_init(VexMutex* m);
void vex_mutex_destroy(VexMutex* m);
int  vex_mutex_lock(VexMutex* m);
int  vex_mutex_trylock(VexMutex* m);
void vex_mutex_unlock(VexMutex* m);
void vex_mutex_poison(VexMutex* m);

void vex_rwlock_init(VexRwLock* rw);
void vex_rwlock_destroy(VexRwLock* rw);
int  vex_rwlock_rdlock(VexRwLock* rw);
int  vex_rwlock_wrlock(VexRwLock* rw);
int  vex_rwlock_tryrdlock(VexRwLock* rw);
int  vex_rwlock_trywrlock(VexRwLock* rw);
void vex_rwlock_unlock(VexRwLock* rw);

void vex_semaphore_init(VexSemaphore* s, uint64_t permits);
void vex_semaphore_destroy(VexSemaphore* s);
void vex_semaphore_acquire(VexSemaphore* s);
int  vex_semaphore_tryacquire(VexSemaphore* s);
void vex_semaphore_release(VexSemaphore* s);

void vex_barrier_init(VexBarrier* b, uint64_t n);
void vex_barrier_destroy(VexBarrier* b);
void vex_barrier_wait(VexBarrier* b);

void vex_once_call(VexOnce* o, void (*fn)(void));

#ifdef __cplusplus
}
#endif
