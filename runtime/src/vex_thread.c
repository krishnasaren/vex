/* vex_thread.c — Threading primitives */
#include "../include/vex_rt.h"
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

typedef struct { void(*fn)(void*); void* arg; } ThreadArg;
static void* thread_entry(void* arg) {
    ThreadArg* t = (ThreadArg*)arg;
    t->fn(t->arg);
    free(t);
    return NULL;
}

VexThread vex_thread_spawn(void(*fn)(void*), void* arg) {
    pthread_t* t = (pthread_t*)vex_alloc(sizeof(pthread_t), 8);
    ThreadArg* ta = (ThreadArg*)malloc(sizeof(ThreadArg));
    ta->fn = fn; ta->arg = arg;
    pthread_create(t, NULL, thread_entry, ta);
    return (VexThread)t;
}

void vex_thread_join(VexThread t) {
    pthread_join(*(pthread_t*)t, NULL);
    vex_free(t, sizeof(pthread_t), 8);
}

void vex_thread_detach(VexThread t) {
    pthread_detach(*(pthread_t*)t);
    vex_free(t, sizeof(pthread_t), 8);
}

void vex_thread_sleep_ms(uint64_t ms) {
    struct timespec ts = { .tv_sec = (time_t)(ms/1000),
                           .tv_nsec = (long)((ms%1000)*1000000) };
    nanosleep(&ts, NULL);
}

VexMutex vex_mutex_new(void) {
    pthread_mutex_t* m = (pthread_mutex_t*)vex_alloc(sizeof(*m), 8);
    pthread_mutex_init(m, NULL);
    return (VexMutex)m;
}
void vex_mutex_lock(VexMutex m)    { pthread_mutex_lock((pthread_mutex_t*)m); }
bool vex_mutex_try_lock(VexMutex m){ return pthread_mutex_trylock((pthread_mutex_t*)m)==0; }
void vex_mutex_unlock(VexMutex m)  { pthread_mutex_unlock((pthread_mutex_t*)m); }
void vex_mutex_free(VexMutex m)    { pthread_mutex_destroy((pthread_mutex_t*)m); vex_free(m,sizeof(pthread_mutex_t),8); }

VexCondVar vex_condvar_new(void) {
    pthread_cond_t* c = (pthread_cond_t*)vex_alloc(sizeof(*c), 8);
    pthread_cond_init(c, NULL);
    return (VexCondVar)c;
}
void vex_condvar_wait(VexCondVar cv, VexMutex m)  { pthread_cond_wait((pthread_cond_t*)cv,(pthread_mutex_t*)m); }
void vex_condvar_notify_one(VexCondVar cv)         { pthread_cond_signal((pthread_cond_t*)cv); }
void vex_condvar_notify_all(VexCondVar cv)         { pthread_cond_broadcast((pthread_cond_t*)cv); }
void vex_condvar_free(VexCondVar cv)               { pthread_cond_destroy((pthread_cond_t*)cv); vex_free(cv,sizeof(pthread_cond_t),8); }
