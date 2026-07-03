#ifndef PATH_TRACER_H
#define PATH_TRACER_H

#include "geometry.h"
#include "scene.h"
#include "vec3.h"

#define MAX_BOUNCES 1024

typedef struct RayTracingArgs {
  Ray3 ray;
  i32 depth;
  f32 light_distance;
  f32 lambda;
} RayTracingArgs;

f32 trace_ray(const Scene *scene, RayTracingArgs *args);

#endif // PATH_TRACER_H
