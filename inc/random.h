#ifndef RANDOM_H
#define RANDOM_H

#include "types.h"

// static _Thread_local u32 global_seed[4] = {257, 263, 269, 271};

static inline u32 rotl(const u32 x, int k) {
  return (x << k) | (x >> (32 - k));
}

static inline u32 random_int(u32 *seeds) {
  const u32 result = seeds[0] + seeds[3];
  const u32 t = seeds[1] << 9;
  seeds[2] ^= seeds[0];
  seeds[3] ^= seeds[1];
  seeds[1] ^= seeds[2];
  seeds[0] ^= seeds[3];
  seeds[2] ^= t;
  seeds[3] = rotl(seeds[3], 11);
  return result;
}

static inline f32 int_to_float(u32 random) {
  union {
    u32 u;
    f32 f;
  } u = {.u = random >> 9 | 0x3f800000};
  return u.f - 1.0;
}

static inline f32 randf(u32 *seeds, f32 a, f32 b) {
  return a + (b - a) * int_to_float(random_int(seeds));
}

#endif // RANDOM_H
