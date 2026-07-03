#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <stdbool.h>

#include "vec3.h"

typedef struct Ray3 {
  Vec3 origin, direction;
} Ray3;

typedef struct IntersectionData {
  Vec3 point, normal;
  f32 distance;
  bool intersects;
  bool front_face;
} IntersectionData;

typedef struct Sphere {
  Vec3 center;
  f32 radius;
  i32 material_index;
} Sphere;

typedef struct Plane {
  Vec3 point, normal;
  i32 material_index;
} Plane;

IntersectionData ray_sphere_hit(Ray3 ray, Sphere sphere);

IntersectionData ray_plane_hit(Ray3 ray, Plane plane);

#endif // GEOMETRY_H
