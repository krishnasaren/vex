/* vex_rt.h — VEX Runtime ABI
 * All runtime functions called from compiled VEX programs.
 * This header is part of the public ABI; do not change function signatures. */
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Memory ───────────────────────────────────────────────────────────────── */
void*    vex_alloc(size_t size, size_t align);
void*    vex_realloc(void* ptr, size_t old_size, size_t new_size, size_t align);
void     vex_free(void* ptr, size_t size, size_t align);
void*    vex_alloc_zeroed(size_t size, size_t align);

/* ── Panic / Assert ───────────────────────────────────────────────────────── */
_Noreturn void vex_panic(const char* msg, size_t msg_len,
                          const char* file, uint32_t line);
_Noreturn void vex_panic_oob(size_t idx, size_t len,
                              const char* file, uint32_t line);
_Noreturn void vex_panic_null(const char* file, uint32_t line);
_Noreturn void vex_panic_overflow(const char* file, uint32_t line);
_Noreturn void vex_unreachable(const char* file, uint32_t line);

/* ── String (UTF-8) ───────────────────────────────────────────────────────── */
typedef struct { const char* ptr; size_t len; } VexStr;
typedef struct { char* ptr; size_t len; size_t cap; } VexString;

VexStr     vex_str_from_cstr(const char* s);
VexStr     vex_str_slice(VexStr s, size_t start, size_t end);
bool       vex_str_eq(VexStr a, VexStr b);
int        vex_str_cmp(VexStr a, VexStr b);
bool       vex_str_contains(VexStr haystack, VexStr needle);
bool       vex_str_starts_with(VexStr s, VexStr prefix);
bool       vex_str_ends_with(VexStr s, VexStr suffix);
VexString  vex_string_new(void);
VexString  vex_string_with_capacity(size_t cap);
void       vex_string_push(VexString* s, VexStr slice);
void       vex_string_push_char(VexString* s, uint32_t codepoint);
VexStr     vex_string_as_str(const VexString* s);
void       vex_string_free(VexString* s);
VexString  vex_string_concat(VexStr a, VexStr b);
VexString  vex_string_repeat(VexStr s, size_t n);

/* ── I/O ──────────────────────────────────────────────────────────────────── */
void   vex_print(VexStr s);
void   vex_println(VexStr s);
void   vex_eprint(VexStr s);
void   vex_eprintln(VexStr s);
void   vex_print_i64(int64_t v);
void   vex_print_u64(uint64_t v);
void   vex_print_f64(double v);
void   vex_print_bool(bool v);
void   vex_print_char(uint32_t cp);
void   vex_flush_stdout(void);
void   vex_flush_stderr(void);

/* Format: vex_format builds a string from a format spec + args (printf-style) */
VexString vex_format(const char* fmt, ...);

/* ── File I/O ─────────────────────────────────────────────────────────────── */
typedef void* VexFile;
VexFile   vex_file_open(VexStr path, uint32_t flags); /* 0=r,1=w,2=rw,3=append */
bool      vex_file_valid(VexFile f);
int64_t   vex_file_read(VexFile f, uint8_t* buf, size_t len);
int64_t   vex_file_write(VexFile f, const uint8_t* buf, size_t len);
void      vex_file_close(VexFile f);
bool      vex_file_exists(VexStr path);
bool      vex_file_delete(VexStr path);

/* ── Array / List (dynamic) ───────────────────────────────────────────────── */
typedef struct { void* ptr; size_t len; size_t cap; size_t elem_size; } VexVec;

VexVec  vex_vec_new(size_t elem_size);
VexVec  vex_vec_with_capacity(size_t elem_size, size_t cap);
void    vex_vec_push(VexVec* v, const void* elem);
void*   vex_vec_get(const VexVec* v, size_t idx);
void    vex_vec_set(VexVec* v, size_t idx, const void* elem);
void    vex_vec_pop(VexVec* v, void* out);
void    vex_vec_insert(VexVec* v, size_t idx, const void* elem);
void    vex_vec_remove(VexVec* v, size_t idx);
void    vex_vec_clear(VexVec* v);
void    vex_vec_free(VexVec* v);
VexVec  vex_vec_clone(const VexVec* v);

/* ── HashMap ──────────────────────────────────────────────────────────────── */
typedef struct VexHashMap_ VexHashMap;
VexHashMap* vex_map_new(size_t key_size, size_t val_size,
                         uint64_t (*hasher)(const void*),
                         bool     (*eq)(const void*, const void*));
void  vex_map_insert(VexHashMap* m, const void* key, const void* val);
void* vex_map_get(const VexHashMap* m, const void* key);
bool  vex_map_remove(VexHashMap* m, const void* key);
size_t vex_map_len(const VexHashMap* m);
void  vex_map_free(VexHashMap* m);

/* ── Result<T, E> ─────────────────────────────────────────────────────────── */
/* Represented as { uint8_t tag; union { T ok; E err; }; }              */
/* tag == 0 → Ok, tag == 1 → Err                                        */
#define VEX_RESULT_OK  0
#define VEX_RESULT_ERR 1

/* ── Option<T> ────────────────────────────────────────────────────────────── */
#define VEX_OPTION_NONE 0
#define VEX_OPTION_SOME 1

/* ── Threading ────────────────────────────────────────────────────────────── */
typedef void* VexThread;
typedef void* VexMutex;
typedef void* VexCondVar;

VexThread vex_thread_spawn(void (*fn)(void*), void* arg);
void      vex_thread_join(VexThread t);
void      vex_thread_detach(VexThread t);
void      vex_thread_sleep_ms(uint64_t ms);

VexMutex  vex_mutex_new(void);
void      vex_mutex_lock(VexMutex m);
bool      vex_mutex_try_lock(VexMutex m);
void      vex_mutex_unlock(VexMutex m);
void      vex_mutex_free(VexMutex m);

VexCondVar vex_condvar_new(void);
void       vex_condvar_wait(VexCondVar cv, VexMutex m);
void       vex_condvar_notify_one(VexCondVar cv);
void       vex_condvar_notify_all(VexCondVar cv);
void       vex_condvar_free(VexCondVar cv);

/* ── Atomics ──────────────────────────────────────────────────────────────── */
int64_t  vex_atomic_load_i64(const int64_t* ptr);
void     vex_atomic_store_i64(int64_t* ptr, int64_t val);
int64_t  vex_atomic_add_i64(int64_t* ptr, int64_t delta);
int64_t  vex_atomic_cas_i64(int64_t* ptr, int64_t expected, int64_t desired);
uint64_t vex_atomic_load_u64(const uint64_t* ptr);
void     vex_atomic_store_u64(uint64_t* ptr, uint64_t val);

/* ── Time ─────────────────────────────────────────────────────────────────── */
uint64_t vex_time_now_ns(void);    /* nanoseconds since epoch */
uint64_t vex_time_now_ms(void);    /* milliseconds since epoch */
void     vex_time_sleep_ms(uint64_t ms);
void     vex_time_sleep_ns(uint64_t ns);

/* ── Math ─────────────────────────────────────────────────────────────────── */
double   vex_math_sqrt(double x);
double   vex_math_pow(double base, double exp);
double   vex_math_log(double x);
double   vex_math_log2(double x);
double   vex_math_log10(double x);
double   vex_math_sin(double x);
double   vex_math_cos(double x);
double   vex_math_tan(double x);
double   vex_math_floor(double x);
double   vex_math_ceil(double x);
double   vex_math_round(double x);
double   vex_math_abs_f64(double x);
int64_t  vex_math_abs_i64(int64_t x);
uint64_t vex_math_min_u64(uint64_t a, uint64_t b);
uint64_t vex_math_max_u64(uint64_t a, uint64_t b);
int64_t  vex_math_min_i64(int64_t a, int64_t b);
int64_t  vex_math_max_i64(int64_t a, int64_t b);
double   vex_math_min_f64(double a, double b);
double   vex_math_max_f64(double a, double b);
uint64_t vex_random_u64(void);
double   vex_random_f64(void);    /* [0.0, 1.0) */

/* ── Process ──────────────────────────────────────────────────────────────── */
_Noreturn void vex_process_exit(int32_t code);
VexStr         vex_env_get(VexStr key);
VexVec         vex_env_args(void);     /* Vec<str> */

/* ── Init / Fini ──────────────────────────────────────────────────────────── */
/* Called from the VEX compiler-generated main() wrapper */
void vex_rt_init(int argc, char** argv);
void vex_rt_fini(void);

#ifdef __cplusplus
}
#endif
