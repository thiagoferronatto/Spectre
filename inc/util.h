#ifndef UTIL_H
#define UTIL_H

#include "types.h"

static inline f32 clampf(f32 x, f32 a, f32 b) {
  return x < a ? a : x > b ? b : x;
}

#endif // UTIL_H
