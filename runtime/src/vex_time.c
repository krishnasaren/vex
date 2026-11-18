/* vex_time.c — Time functions */
#include "../include/vex_rt.h"
#include <time.h>

uint64_t vex_time_now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}
uint64_t vex_time_now_ms(void) { return vex_time_now_ns() / 1000000ULL; }
void vex_time_sleep_ms(uint64_t ms) { vex_thread_sleep_ms(ms); }
void vex_time_sleep_ns(uint64_t ns) {
    struct timespec ts = { .tv_sec=(time_t)(ns/1000000000ULL), .tv_nsec=(long)(ns%1000000000ULL) };
    nanosleep(&ts, NULL);
}
