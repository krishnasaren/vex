/* vex_math.c — Math runtime */
#include "../include/vex_rt.h"
#include <math.h>
#include <stdlib.h>
#include <stdint.h>

double vex_math_sqrt(double x)  { return sqrt(x); }
double vex_math_pow(double b, double e) { return pow(b,e); }
double vex_math_log(double x)   { return log(x); }
double vex_math_log2(double x)  { return log2(x); }
double vex_math_log10(double x) { return log10(x); }
double vex_math_sin(double x)   { return sin(x); }
double vex_math_cos(double x)   { return cos(x); }
double vex_math_tan(double x)   { return tan(x); }
double vex_math_floor(double x) { return floor(x); }
double vex_math_ceil(double x)  { return ceil(x); }
double vex_math_round(double x) { return round(x); }
double vex_math_abs_f64(double x) { return fabs(x); }
int64_t  vex_math_abs_i64(int64_t x) { return x<0?-x:x; }
uint64_t vex_math_min_u64(uint64_t a,uint64_t b) { return a<b?a:b; }
uint64_t vex_math_max_u64(uint64_t a,uint64_t b) { return a>b?a:b; }
int64_t  vex_math_min_i64(int64_t a,int64_t b)   { return a<b?a:b; }
int64_t  vex_math_max_i64(int64_t a,int64_t b)   { return a>b?a:b; }
double   vex_math_min_f64(double a,double b)       { return a<b?a:b; }
double   vex_math_max_f64(double a,double b)       { return a>b?a:b; }

static uint64_t _rand_state = 0x853c49e6748fea9bULL;
uint64_t vex_random_u64(void) {
    _rand_state ^= _rand_state >> 12;
    _rand_state ^= _rand_state << 25;
    _rand_state ^= _rand_state >> 27;
    return _rand_state * 0x2545f4914f6cdd1dULL;
}
double vex_random_f64(void) {
    return (double)(vex_random_u64() >> 11) / (double)(1ULL<<53);
}
