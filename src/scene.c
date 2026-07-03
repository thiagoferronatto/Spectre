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
  materials[0] = (Material){.shine = 1600, .color = SPD_RED, .ior = 1.5};
  materials[1] = (Material){.shine = 1200, .color = SPD_GREEN, .ior = 1.5};
  materials[2] = (Material){.shine = 800, .color = SPD_BLUE, .ior = 1.5};
  materials[3] = (Material){.shine = 400, .color = SPD_CYAN, .ior = 1.5};
  materials[4] = (Material){.shine = 200, .color = SPD_MAGENTA, .ior = 1.5};
  materials[5] = (Material){.shine = 50, .color = SPD_YELLOW, .ior = 1.5};
  materials[6] = (Material){.shine = 50, .color = SPD_CHECKERS_XZ, .ior = 1.5};
  materials[7] = (Material){.shine = 1600, .color = SPD_RED, .ior = 1.5};
  materials[7].transmissive = true;
  materials[8] = (Material){.shine = 1600, .color = SPD_BLACK, .ior = 1.5};

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
  *camera = custom_camera(36, 36, 85);

  scene.camera = camera;
  scene.light_pos = (Vec3){-2, 1, -1};
  scene.light_intensity = 7.5 * M_PI;
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
