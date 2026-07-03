#ifndef SCENE_H
#define SCENE_H

#include <stdbool.h>

#include "camera.h"
#include "color.h"
#include "geometry.h"
#include "vec3.h"

typedef struct Material {
  SPD color;
  f32 shine;
  f32 ior;
  bool transmissive;
} Material;

// there's only one of these so packing and aliasing don't matter
typedef struct Scene {
  Camera *camera;

  Vec3 light_pos; // change into a lights array

  SPD light_color;
  SPD ambient_color;

  Material *materials;

  Sphere *spheres;
  Plane *planes;

  // other shape pointers go here...

  i32 sphere_count;
  i32 plane_count;

  // other shape counts go here...

  f32 light_intensity;
  f32 light_r;
} Scene;

Scene default_scene(void);

void free_scene(Scene *scene);

#endif // SCENE_H
