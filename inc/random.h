#ifndef RANDOM_H
#define RANDOM_H

#include "types.h"

static _Thread_local u32 global_seed[4] = {257, 263, 269, 271};

static inline u32 rotl(const u32 x, int k) {
  return (x << k) | (x >> (32 - k));
}

static inline u32 random_int(void) {
  const u32 result = global_seed[0] + global_seed[3];
  const u32 t = global_seed[1] << 9;
  global_seed[2] ^= global_seed[0];
  global_seed[3] ^= global_seed[1];
  global_seed[1] ^= global_seed[2];
  global_seed[0] ^= global_seed[3];
  global_seed[2] ^= t;
  global_seed[3] = rotl(global_seed[3], 11);
  return result;
}

static inline f32 int_to_float(u32 random) {
  union {
    u32 u;
    f32 f;
  } u = {.u = random >> 9 | 0x3f800000};
  return u.f - 1.0;
}

static inline f32 randf(f32 a, f32 b) {
  return a + (b - a) * int_to_float(random_int());
}

#endif // RANDOM_H
