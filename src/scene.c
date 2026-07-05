#include "scene.h"

#include <stdlib.h>

#define _USE_MATH_DEFINES
#include <math.h>

/// @brief Generates a default scene with objects, a light and a camera.
/// @return A Scene object.
Scene default_scene(void) {
  Scene scene;

  const i32 material_count = 9;
  Material *materials = malloc(material_count * sizeof(Material));
  materials[0] = (Material){
      .roughness = 0, .color = SPD_RED, .ior_A = 1.5, .ior_B = 0.00354};
  materials[1] = (Material){
      .roughness = 0.01, .color = SPD_GREEN, .ior_A = 1.5, .ior_B = 0.00354};
  materials[2] = (Material){
      .roughness = 0.05, .color = SPD_BLUE, .ior_A = 1.5, .ior_B = 0.00354};
  materials[3] = (Material){
      .roughness = 0.1, .color = SPD_CYAN, .ior_A = 1.5, .ior_B = 0.00354};
  materials[4] = (Material){
      .roughness = 0.25, .color = SPD_MAGENTA, .ior_A = 1.5, .ior_B = 0.00354};
  materials[5] = (Material){
      .roughness = 0.5, .color = SPD_YELLOW, .ior_A = 1.5, .ior_B = 0.00354};
  materials[6] = (Material){
      .roughness = 1, .color = SPD_CHECKERS_XZ, .ior_A = 1.5, .ior_B = 0.00354};
  materials[7] = (Material){
      .roughness = 0, .color = SPD_RED, .ior_A = 1.318181818, .ior_B = 0.1};
  materials[7].transmissive = true;
  materials[8] = (Material){
      .roughness = 1, .color = SPD_BLACK, .ior_A = 1.5, .ior_B = 0.00354};

  Sphere *spheres = malloc(17 * sizeof(Sphere));
  i32 mtl_idx = 0;
  for (i32 i = 0; i < 4; ++i) {
    for (i32 j = 0; j < 4; ++j) {
      spheres[i * 4 + j] = (Sphere){.center = {0.25 * j - 0.375, //
                                               -0.3 + 0.2 * i,   //
                                               -0.75 - 0.21 * i},
                                    .radius = 0.1,
                                    .material_index = mtl_idx};
      mtl_idx = (mtl_idx + 1) % (material_count - 3);
    }
  }
  spheres[16] =
      (Sphere){.center = {0, 0, -0.5}, .radius = 0.1, .material_index = 7};

  Plane *planes = malloc(1 * sizeof(Plane));
  planes[0] = (Plane){.point = (Vec3){0, -0.4, 0},
                      .normal = (Vec3){0, 1, 0},
                      .material_index = 6};

  Camera *camera = malloc(sizeof(Camera));
  *camera = full_frame(18);
  camera->focal_distance = 0.355;
  //   camera->f_number = 0.1;

  scene.camera = camera;
  scene.light_pos = (Vec3){-2, 1, -1};
  scene.light_intensity = 50;
  scene.light_r = 0.1;
  scene.light_color = SPD_ILL_E;
  scene.ambient_color = SPD_ILL_AMBIENT;
  scene.materials = materials;
  scene.spheres = spheres;
  scene.planes = planes;
  scene.sphere_count = 17;
  scene.plane_count = 1;

  return scene;
}

void free_scene(Scene *scene) {
  free(scene->camera);
  free(scene->materials);
  free(scene->spheres);
  free(scene->planes);
}
