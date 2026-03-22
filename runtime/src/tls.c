#include "vex_rt/tls.h"
#include "vex_rt/alloc.h"
#include "vex_rt/panic.h"

VexTlsKey vex_tls_create(VexTlsDestructor dtor) {
    VexTlsKey k;
    if (pthread_key_create(&k.inner, dtor) != 0)
        vex_panic("vex_tls_create failed", __FILE__, __LINE__);
    return k;
}
void  vex_tls_set(VexTlsKey k, void* val)  { pthread_setspecific(k.inner, val); }
void* vex_tls_get(VexTlsKey k)             { return pthread_getspecific(k.inner); }
void  vex_tls_destroy(VexTlsKey k)         { pthread_key_delete(k.inner); }
