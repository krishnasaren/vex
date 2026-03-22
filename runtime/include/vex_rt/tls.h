#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include <pthread.h>
typedef void (*VexTlsDestructor)(void*);
typedef struct { pthread_key_t inner; } VexTlsKey;
VexTlsKey vex_tls_create(VexTlsDestructor dtor);
void      vex_tls_set(VexTlsKey k, void* val);
void*     vex_tls_get(VexTlsKey k);
void      vex_tls_destroy(VexTlsKey k);
#ifdef __cplusplus
}
#endif
