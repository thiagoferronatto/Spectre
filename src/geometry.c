#include "geometry.h"

#include <stdbool.h>

IntersectionData ray_plane_hit(Ray3 ray, Plane plane) {
  f32 d_dot_n = vec3_dot(ray.direction, plane.normal);
  if (d_dot_n == 0)
    return (IntersectionData){.intersects = false};
  f32 t = vec3_dot(vec3_sub(plane.point, ray.origin), plane.normal) / d_dot_n;
  if (t < 0)
    return (IntersectionData){.intersects = false};
  IntersectionData hit;
  hit.point = vec3_add(ray.origin, vec3_muls(ray.direction, t));
  hit.normal = plane.normal;
  hit.distance = t;
  hit.intersects = true;
  hit.front_face = d_dot_n < 0;
  return hit;
}

IntersectionData ray_sphere_hit(Ray3 ray, Sphere sphere) {
  Vec3 oc = vec3_sub(ray.origin, sphere.center);
  f32 oc2 = vec3_dot(oc, oc);
  f32 half_b = vec3_dot(oc, ray.direction);
  f32 r = sphere.radius;
  f32 discriminant = half_b * half_b - oc2 + r * r;
  if (discriminant < 0)
    return (IntersectionData){.intersects = false};
  f32 t0 = -half_b - sqrtf(discriminant);
  f32 t1 = -half_b + sqrtf(discriminant);
  f32 t = t0 < 0.0f ? t1 : t0;
  if (t < 0.0f)
    return (IntersectionData){.intersects = false};
  Vec3 point = vec3_add(ray.origin, vec3_muls(ray.direction, t));
  Vec3 normal = vec3_divs(vec3_sub(point, sphere.center), r);
  IntersectionData hit;
  hit.point = point;
  hit.normal = normal;
  hit.distance = t;
  hit.intersects = true;
  hit.front_face = vec3_dot(ray.direction, normal) < 0;
  return hit;
}
