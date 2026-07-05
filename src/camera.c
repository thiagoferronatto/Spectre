#include "camera.h"

f32 entrance_pupil_radius(const Camera *camera) {
  return 0.5 * camera->focal_length / camera->f_number;
}

Camera full_frame(f32 focal_length_mm) {
  return custom_camera(36, 24, focal_length_mm);
}

Camera custom_camera(f32 sensor_width_mm, f32 sensor_height_mm,
                     f32 focal_length_mm) {
  Camera camera;
  camera.position = (Vec3){0, 0, 0};
  camera.focal_length = 0.001 * focal_length_mm;
  camera.sensor_width = 0.001 * sensor_width_mm;
  camera.sensor_height = 0.001 * sensor_height_mm;
  camera.focal_distance = 0.75;
  camera.f_number = 1.8;
  return camera;
}

Vec3 pixel_dimensions(const Camera *camera, i32 width, i32 height) {
  return (Vec3){camera->sensor_width / width,   //
                camera->sensor_height / height, //
                1};
}
