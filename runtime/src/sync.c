// vex_rt/sync.c — Mutex, RwLock, Semaphore, Barrier, Once

#include "vex_rt/sync.h"
#include "vex_rt/alloc.h"
#include "vex_rt/panic.h"
#include <string.h>

/* ---------- Mutex ---------- */
void vex_mutex_init(VexMutex* m)    { pthread_mutex_init(&m->inner, NULL); m->poisoned = 0; }
void vex_mutex_destroy(VexMutex* m) { pthread_mutex_destroy(&m->inner); }

int vex_mutex_lock(VexMutex* m) {
    pthread_mutex_lock(&m->inner);
    return m->poisoned ? -1 : 0;   /* -1 = poisoned */
}

int vex_mutex_trylock(VexMutex* m) {
    int r = pthread_mutex_trylock(&m->inner);
    if (r != 0) return -2;          /* -2 = already locked */
    return m->poisoned ? -1 : 0;
}

void vex_mutex_unlock(VexMutex* m) { pthread_mutex_unlock(&m->inner); }
void vex_mutex_poison(VexMutex* m) { m->poisoned = 1; }

/* ---------- RwLock ---------- */
void vex_rwlock_init(VexRwLock* rw)    { pthread_rwlock_init(&rw->inner, NULL); rw->poisoned = 0; }
void vex_rwlock_destroy(VexRwLock* rw) { pthread_rwlock_destroy(&rw->inner); }

int  vex_rwlock_rdlock(VexRwLock* rw)  { pthread_rwlock_rdlock(&rw->inner);  return rw->poisoned ? -1 : 0; }
int  vex_rwlock_wrlock(VexRwLock* rw)  { pthread_rwlock_wrlock(&rw->inner);  return rw->poisoned ? -1 : 0; }
int  vex_rwlock_tryrdlock(VexRwLock* rw) { return pthread_rwlock_tryrdlock(&rw->inner) == 0 ? 0 : -2; }
int  vex_rwlock_trywrlock(VexRwLock* rw) { return pthread_rwlock_trywrlock(&rw->inner) == 0 ? 0 : -2; }
void vex_rwlock_unlock(VexRwLock* rw)  { pthread_rwlock_unlock(&rw->inner); }

/* ---------- Semaphore ---------- */
void vex_semaphore_init(VexSemaphore* s, uint64_t permits) {
    pthread_mutex_init(&s->mutex, NULL);
    pthread_cond_init(&s->cond, NULL);
    s->count = permits;
}
void vex_semaphore_destroy(VexSemaphore* s) {
    pthread_mutex_destroy(&s->mutex);
    pthread_cond_destroy(&s->cond);
}
void vex_semaphore_acquire(VexSemaphore* s) {
    pthread_mutex_lock(&s->mutex);
    while (s->count == 0) pthread_cond_wait(&s->cond, &s->mutex);
    s->count--;
    pthread_mutex_unlock(&s->mutex);
}
int vex_semaphore_tryacquire(VexSemaphore* s) {
    pthread_mutex_lock(&s->mutex);
    int ok = (s->count > 0);
    if (ok) s->count--;
    pthread_mutex_unlock(&s->mutex);
    return ok;
}
void vex_semaphore_release(VexSemaphore* s) {
    pthread_mutex_lock(&s->mutex);
    s->count++;
    pthread_cond_signal(&s->cond);
    pthread_mutex_unlock(&s->mutex);
}

/* ---------- Barrier ---------- */
void vex_barrier_init(VexBarrier* b, uint64_t n) { pthread_barrier_init(&b->inner, NULL, (unsigned)n); }
void vex_barrier_destroy(VexBarrier* b) { pthread_barrier_destroy(&b->inner); }
void vex_barrier_wait(VexBarrier* b)    { pthread_barrier_wait(&b->inner); }

/* ---------- Once ---------- */
void vex_once_call(VexOnce* o, void (*fn)(void)) { pthread_once(&o->inner, fn); }
