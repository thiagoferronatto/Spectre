#ifndef VEC3_H
#define VEC3_H

#include <math.h>

#include "types.h"
#include "util.h"

typedef struct Vec3 {
  union {
    struct {
      f32 x, y, z;
    };
    f32 xyz[3];
  };
} Vec3;

typedef struct PixelRgb {
  u8 x, y, z;
} PixelRgb;

static inline Vec3 vec3_add(Vec3 a, Vec3 b) {
  return (Vec3){a.x + b.x, a.y + b.y, a.z + b.z};
}

static inline Vec3 vec3_adds(Vec3 a, f32 x) {
  return (Vec3){a.x + x, a.y + x, a.z + x};
}

static inline Vec3 vec3_muls(Vec3 a, f32 s) {
  return (Vec3){s * a.x, s * a.y, s * a.z};
}

static inline Vec3 vec3_mul(Vec3 a, Vec3 b) {
  return (Vec3){a.x * b.x, a.y * b.y, a.z * b.z};
}

static inline Vec3 vec3_div(Vec3 a, Vec3 b) {
  return (Vec3){a.x / b.x, a.y / b.y, a.z / b.z};
}

static inline Vec3 vec3_divs(Vec3 a, f32 s) {
  float inv = 1.0f / s;
  return (Vec3){inv * a.x, inv * a.y, inv * a.z};
}

static inline Vec3 vec3_neg(Vec3 v) { return (Vec3){-v.x, -v.y, -v.z}; }

static inline Vec3 vec3_sub(Vec3 a, Vec3 b) {
  return (Vec3){a.x - b.x, a.y - b.y, a.z - b.z};
}

static inline Vec3 vec3_subs(Vec3 a, f32 s) {
  return (Vec3){a.x - s, a.y - s, a.z - s};
}

static inline f32 vec3_dot(Vec3 a, Vec3 b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline f32 vec3_sqlen(Vec3 v) {
  return v.x * v.x + v.y * v.y + v.z * v.z;
}

static inline f32 vec3_len(Vec3 v) { return sqrtf(vec3_sqlen(v)); }

static inline Vec3 vec3_normalize(Vec3 v) { return vec3_divs(v, vec3_len(v)); }

static inline Vec3 vec3_clamp(Vec3 v, f32 a, f32 b) {
  return (Vec3){clampf(v.x, a, b), clampf(v.y, a, b), clampf(v.z, a, b)};
}

static inline Vec3 vec3_maxs(Vec3 v, f32 x) {
  return (Vec3){fmaxf(v.x, x), fmaxf(v.y, x), fmaxf(v.z, x)};
}

static inline Vec3 vec3_max(Vec3 a, Vec3 b) {
  return (Vec3){fmaxf(a.x, b.x), fmaxf(a.y, b.y), fmaxf(a.z, b.z)};
}

static inline Vec3 vec3_min(Vec3 a, Vec3 b) {
  return (Vec3){fminf(a.x, b.x), fminf(a.y, b.y), fminf(a.z, b.z)};
}

static inline Vec3 vec3_pows(Vec3 v, f32 x) {
  return (Vec3){powf(v.x, x), powf(v.y, x), powf(v.z, x)};
}

static inline Vec3 vec3_round(Vec3 v) {
  return (Vec3){roundf(v.x), roundf(v.y), roundf(v.z)};
}

#endif // VEC3_H
