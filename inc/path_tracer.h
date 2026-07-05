#ifndef PATH_TRACER_H
#define PATH_TRACER_H

#define _USE_MATH_DEFINES
#include <math.h>

#include "geometry.h"
#include "random.h"
#include "scene.h"
#include "util.h"
#include "vec3.h"

#define MAX_BOUNCES 1024

typedef struct RayTracingArgs {
  Ray3 ray;
  i32 depth;
  f32 light_distance;
  f32 lambda;
} RayTracingArgs;

static Vec3 shift_light(u32 *seeds, Vec3 light_pos, f32 light_radius) {
  if (light_radius == 0)
    return light_pos;
  Vec3 shift;
  f32 light_radius_sq = light_radius * light_radius;
  do {
    shift.x = randf(seeds, -light_radius, light_radius);
    shift.y = randf(seeds, -light_radius, light_radius);
    shift.z = randf(seeds, -light_radius, light_radius);
  } while (vec3_dot(shift, shift) > light_radius_sq);
  shift = vec3_muls(vec3_normalize(shift), light_radius);
  return vec3_add(light_pos, shift);
}

static bool trace_shadow_ray(const Scene *scene, RayTracingArgs *args) {
  for (i32 i = 0; i < scene->sphere_count; ++i) {
    IntersectionData hit = ray_sphere_hit(args->ray, scene->spheres[i]);
    if (hit.intersects && hit.distance < args->light_distance) {
      // Material *mtl = scene->materials + scene->spheres[i].material_index;
      // if (mtl->transmissive)
      //   continue;
      return true;
    }
  }
  for (i32 i = 0; i < scene->plane_count; ++i) {
    IntersectionData hit = ray_plane_hit(args->ray, scene->planes[i]);
    if (hit.intersects && hit.distance < args->light_distance)
      return true;
  }
  return false;
}

static inline f32 trace_ray(u32 *seeds, const Scene *scene,
                            RayTracingArgs *args) {
  if (args->depth == MAX_BOUNCES)
    return 0;

  // geometry/ray tracing
  IntersectionData closest_hit = {.distance = 1e10, .intersects = false};
  i32 sphere_index = -1;
  i32 plane_index = -1;
  for (i32 i = 0; i < scene->sphere_count; ++i) {
    IntersectionData hit = ray_sphere_hit(args->ray, scene->spheres[i]);
    if (!hit.intersects || hit.distance >= closest_hit.distance)
      continue;
    closest_hit = hit;
    sphere_index = i;
    plane_index = -1;
  }
  for (i32 i = 0; i < scene->plane_count; ++i) {
    IntersectionData hit = ray_plane_hit(args->ray, scene->planes[i]);
    if (!hit.intersects || hit.distance >= closest_hit.distance)
      continue;
    closest_hit = hit;
    plane_index = i;
    sphere_index = -1;
  }

  if (!closest_hit.intersects)
    return lambda_response(args->lambda, SPD_ILL_AMBIENT, (Vec3){0});

  Vec3 normal = closest_hit.normal;
  if (!closest_hit.front_face)
    normal = vec3_neg(normal);
  Vec3 point = vec3_add(closest_hit.point, vec3_muls(normal, 1e-5));
  i32 material_index;
  if (sphere_index != -1)
    material_index = scene->spheres[sphere_index].material_index;
  else
    material_index = scene->planes[plane_index].material_index;
  Material *material = scene->materials + material_index;

  RayTracingArgs shadow_ray_args = *args;
  Vec3 light_pos = shift_light(seeds, scene->light_pos, scene->light_r);
  Vec3 shadow_ray_dir = vec3_sub(light_pos, point);
  f32 light_distance = vec3_len(shadow_ray_dir);
  Vec3 unit_shadow_ray_dir = vec3_divs(shadow_ray_dir, light_distance);
  shadow_ray_args.ray.origin = point;
  shadow_ray_args.ray.direction = unit_shadow_ray_dir;
  shadow_ray_args.light_distance = light_distance;
  bool in_shadow = trace_shadow_ray(scene, &shadow_ray_args);

  // lighting
  f32 d_dot_n = vec3_dot(args->ray.direction, normal);

  // cauchy's equation for IOR
  f32 lambda_um = args->lambda * 0.001f; // cauchy's equation is in micrometers
  f32 ior = material->ior_A + material->ior_B / (lambda_um * lambda_um);
  if (closest_hit.front_face)
    ior = 1.0f / ior;

  // dielectric fresnel equations
  f32 fresnel;
  f32 cos_i = fabsf(d_dot_n);
  f32 sin_i2 = fmaxf(0.0f, 1.0f - cos_i * cos_i);
  f32 sin_t2 = sin_i2 * (ior * ior);
  if (sin_t2 < 1.0f) {
    f32 cos_t = sqrtf(fmaxf(0.0f, 1.0f - sin_t2));
    f32 r_s = (ior * cos_i - cos_t) / (ior * cos_i + cos_t);
    f32 r_p = (ior * cos_t - cos_i) / (ior * cos_t + cos_i);
    fresnel = 0.5f * (r_s * r_s + r_p * r_p);
  } else {
    fresnel = 1.0f;
  }
  // temporary fix for ugly highlight on the floor
  if (material->roughness == 1)
    fresnel = 0;

  // direct (always compute, no need for extra rays)
  f32 direct_light = 0;
  if (!in_shadow) {
    light_pos = shift_light(seeds, scene->light_pos, scene->light_r);
    Vec3 light_vec = vec3_sub(light_pos, point);
    Vec3 view_dir = vec3_neg(args->ray.direction);
    f32 inv_light_dist = 1 / vec3_len(light_vec);
    f32 attenuation = inv_light_dist * inv_light_dist;
    f32 effective_intensity = scene->light_intensity * attenuation;
    Vec3 light_dir = vec3_muls(light_vec, inv_light_dist);
    Vec3 half_dir = vec3_normalize(vec3_add(light_dir, view_dir));
    f32 n_dot_l = fmaxf(vec3_dot(normal, light_dir), 0);
    f32 n_dot_h = clampf(vec3_dot(normal, half_dir), 0, 1);

    f32 n_dot_v = fmaxf(vec3_dot(normal, view_dir), 0.0f);
    n_dot_l = fmaxf(n_dot_l, 0.0f);
    f32 r = clampf(material->roughness, 0.001, 0.999);
    f32 r2 = r * r;
    f32 A = 1.0f - 0.5f * (r2 / (r2 + 0.57f));
    f32 B = 0.45f * (r2 / (r2 + 0.09f));
    f32 v_dot_l = vec3_dot(view_dir, light_dir);
    f32 cx = v_dot_l - (n_dot_v * n_dot_l);
    f32 gamma_C = fmaxf(0.0f, cx) / fmaxf(fmaxf(n_dot_v, n_dot_l), 1e-6f);
    f32 lr = n_dot_l * (A + B * gamma_C);
    f32 diffuse_factor = lr;
    diffuse_factor *= (1 - fresnel);
    f32 diffuse_term = lambda_response(args->lambda, material->color, point);
    diffuse_term *= diffuse_factor;

    f32 nh2 = n_dot_h * n_dot_h;
    f32 denom = nh2 * r2 - nh2 + 1;
    f32 d = r2 / (M_PI * denom * denom);
    f32 nl2 = n_dot_l * n_dot_l, nv2 = n_dot_v * n_dot_v;
    f32 g1 = (2 * n_dot_l) / (n_dot_l + sqrtf(r2 + (1 - r2) * nl2));
    f32 g2 = (2 * n_dot_v) / (n_dot_v + sqrtf(r2 + (1 - r2) * nv2));
    f32 g = g1 * g2;
    f32 spec = fresnel * g * d / (M_PI * n_dot_v);
    f32 surface_reflection = spec;

    if (!material->transmissive)
      surface_reflection += diffuse_term;
    f32 response = lambda_response(args->lambda, scene->light_color, point);
    f32 unattenuated_light = surface_reflection * response;
    direct_light = unattenuated_light * effective_intensity;
  }

  f32 indirect_light = 0;
  f32 bounce_light = 0;
  f32 refracted_light = 0;
  if (randf(seeds, 0, 1) < fresnel) {
    // reflections (indirect specular, compute by chance)
    Vec3 twice_projection = vec3_muls(normal, 2 * d_dot_n);
    Vec3 reflection_dir = vec3_sub(args->ray.direction, twice_projection);
    f32 reflection_error = material->roughness;
    Vec3 shift;
    do {
      shift.x = randf(seeds, -1, 1);
      shift.y = randf(seeds, -1, 1);
      shift.z = randf(seeds, -1, 1);
    } while (vec3_dot(shift, shift) > 1);
    shift = vec3_muls(shift, reflection_error);
    reflection_dir = vec3_normalize(vec3_add(reflection_dir, shift));
    f32 r_dot_n = vec3_dot(reflection_dir, normal);
    if (r_dot_n < 0)
      reflection_dir = vec3_sub(reflection_dir, vec3_muls(normal, 2 * r_dot_n));
    Ray3 new_ray = {.origin = point, .direction = reflection_dir};
    RayTracingArgs next_args = *args;
    next_args.ray = new_ray;
    ++next_args.depth;
    indirect_light = trace_ray(seeds, scene, &next_args);
  } else {
    if (material->transmissive) {
      RayTracingArgs refraction_args = *args;
      Vec3 point = vec3_add(closest_hit.point, vec3_muls(normal, -1e-5));
      refraction_args.ray.origin = point;
      f32 sin_2_theta_i = fmaxf(0, 1 - d_dot_n * d_dot_n);
      f32 sin_2_theta_t = sin_2_theta_i * (ior * ior);
      f32 cos_theta_t = sqrtf(fmaxf(0, 1 - sin_2_theta_t));
      Vec3 refr_dir = vec3_add(vec3_muls(args->ray.direction, ior),
                               vec3_muls(normal, -ior * d_dot_n - cos_theta_t));
      refr_dir = vec3_normalize(refr_dir);
      refraction_args.ray.direction = refr_dir;
      ++refraction_args.depth;
      refracted_light = trace_ray(seeds, scene, &refraction_args);
    } else {
      // bounce lighting (indirect diffuse, compute by chance)
      Vec3 indirect_dir;
      do {
        indirect_dir.x = randf(seeds, -1, 1);
        indirect_dir.y = randf(seeds, -1, 1);
        indirect_dir.z = randf(seeds, -1, 1);
      } while (vec3_dot(indirect_dir, indirect_dir) > 1);
      indirect_dir = vec3_normalize(indirect_dir);
      indirect_dir = vec3_normalize(vec3_add(indirect_dir, normal));
      RayTracingArgs indirect_args = *args;
      indirect_args.ray = (Ray3){.origin = point, .direction = indirect_dir};
      ++indirect_args.depth;
      f32 indirect_result = trace_ray(seeds, scene, &indirect_args);
      f32 response = lambda_response(args->lambda, material->color, point);
      bounce_light = indirect_result * response;
    }
  }

  f32 out = direct_light + indirect_light + bounce_light + refracted_light;
  return out;
}

#endif // PATH_TRACER_H
