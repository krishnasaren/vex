// vex_rt/channel.c — MPSC channel (multi-producer, single-consumer)

#include "vex_rt/channel.h"
#include "vex_rt/alloc.h"
#include "vex_rt/panic.h"
#include <string.h>

/* Circular-buffer bounded channel */
VexChannel* vex_channel_create(uint64_t capacity, uint64_t item_size) {
    if (capacity == 0) capacity = 1;
    VexChannel* ch = (VexChannel*)vex_alloc(sizeof(VexChannel), _Alignof(VexChannel));
    ch->buf        = (char*)vex_alloc(capacity * item_size, 16);
    ch->capacity   = capacity;
    ch->item_size  = item_size;
    ch->head       = 0;
    ch->tail       = 0;
    ch->count      = 0;
    ch->closed     = 0;
    pthread_mutex_init(&ch->mutex, NULL);
    pthread_cond_init(&ch->not_empty, NULL);
    pthread_cond_init(&ch->not_full, NULL);
    return ch;
}

void vex_channel_destroy(VexChannel* ch) {
    if (!ch) return;
    pthread_mutex_destroy(&ch->mutex);
    pthread_cond_destroy(&ch->not_empty);
    pthread_cond_destroy(&ch->not_full);
    vex_free(ch->buf);
    vex_free(ch);
}

/* Returns 1 on success, 0 if channel closed */
int vex_channel_send(VexChannel* ch, const void* item) {
    pthread_mutex_lock(&ch->mutex);
    while (ch->count == ch->capacity && !ch->closed)
        pthread_cond_wait(&ch->not_full, &ch->mutex);
    if (ch->closed) { pthread_mutex_unlock(&ch->mutex); return 0; }
    memcpy(ch->buf + ch->tail * ch->item_size, item, ch->item_size);
    ch->tail = (ch->tail + 1) % ch->capacity;
    ch->count++;
    pthread_cond_signal(&ch->not_empty);
    pthread_mutex_unlock(&ch->mutex);
    return 1;
}

/* Non-blocking send. Returns 1 sent, 0 full, -1 closed */
int vex_channel_trysend(VexChannel* ch, const void* item) {
    pthread_mutex_lock(&ch->mutex);
    if (ch->closed) { pthread_mutex_unlock(&ch->mutex); return -1; }
    if (ch->count == ch->capacity) { pthread_mutex_unlock(&ch->mutex); return 0; }
    memcpy(ch->buf + ch->tail * ch->item_size, item, ch->item_size);
    ch->tail = (ch->tail + 1) % ch->capacity;
    ch->count++;
    pthread_cond_signal(&ch->not_empty);
    pthread_mutex_unlock(&ch->mutex);
    return 1;
}

/* Returns 1 on success, 0 if closed and empty */
int vex_channel_recv(VexChannel* ch, void* out) {
    pthread_mutex_lock(&ch->mutex);
    while (ch->count == 0 && !ch->closed)
        pthread_cond_wait(&ch->not_empty, &ch->mutex);
    if (ch->count == 0) { pthread_mutex_unlock(&ch->mutex); return 0; }
    memcpy(out, ch->buf + ch->head * ch->item_size, ch->item_size);
    ch->head = (ch->head + 1) % ch->capacity;
    ch->count--;
    pthread_cond_signal(&ch->not_full);
    pthread_mutex_unlock(&ch->mutex);
    return 1;
}

/* Non-blocking recv. Returns 1 received, 0 empty, -1 closed+empty */
int vex_channel_tryrecv(VexChannel* ch, void* out) {
    pthread_mutex_lock(&ch->mutex);
    if (ch->count == 0) {
        int r = ch->closed ? -1 : 0;
        pthread_mutex_unlock(&ch->mutex);
        return r;
    }
    memcpy(out, ch->buf + ch->head * ch->item_size, ch->item_size);
    ch->head = (ch->head + 1) % ch->capacity;
    ch->count--;
    pthread_cond_signal(&ch->not_full);
    pthread_mutex_unlock(&ch->mutex);
    return 1;
}

void vex_channel_close(VexChannel* ch) {
    pthread_mutex_lock(&ch->mutex);
    ch->closed = 1;
    pthread_cond_broadcast(&ch->not_empty);
    pthread_cond_broadcast(&ch->not_full);
    pthread_mutex_unlock(&ch->mutex);
}

uint64_t vex_channel_len(VexChannel* ch) {
    pthread_mutex_lock(&ch->mutex);
    uint64_t n = ch->count;
    pthread_mutex_unlock(&ch->mutex);
    return n;
}
