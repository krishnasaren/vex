#pragma once
#ifdef __cplusplus
extern "C" {
#endif
// Future trait interface — implemented per future type
typedef struct VexFutureVtable {
    int  (*poll)(void* state, void* output, void* waker);
    void (*drop)(void* state);
} VexFutureVtable;

typedef struct VexFuture {
    const VexFutureVtable* vtable;
    void*                  state;
} VexFuture;

int  vex_future_poll(VexFuture* f, void* output, void* waker);
void vex_future_drop(VexFuture* f);
#ifdef __cplusplus
}
#endif
