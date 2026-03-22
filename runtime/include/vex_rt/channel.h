#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include <pthread.h>

typedef struct {
    char*           buf;
    uint64_t        capacity;
    uint64_t        item_size;
    uint64_t        head;
    uint64_t        tail;
    uint64_t        count;
    int             closed;
    pthread_mutex_t mutex;
    pthread_cond_t  not_empty;
    pthread_cond_t  not_full;
} VexChannel;

VexChannel* vex_channel_create(uint64_t capacity, uint64_t item_size);
void        vex_channel_destroy(VexChannel* ch);
int         vex_channel_send(VexChannel* ch, const void* item);
int         vex_channel_trysend(VexChannel* ch, const void* item);
int         vex_channel_recv(VexChannel* ch, void* out);
int         vex_channel_tryrecv(VexChannel* ch, void* out);
void        vex_channel_close(VexChannel* ch);
uint64_t    vex_channel_len(VexChannel* ch);

#ifdef __cplusplus
}
#endif
