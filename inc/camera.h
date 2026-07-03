#ifndef CAMERA_H
#define CAMERA_H

#include "vec3.h"

typedef struct Camera {
  Vec3 position;
  f32 sensor_width;
  f32 sensor_height;
  f32 focal_length;
  f32 f_number;
  f32 focal_distance;
} Camera;

f32 entrance_pupil_radius(const Camera *camera);

Camera full_frame(f32 focal_length_mm);

Camera custom_camera(f32 sensor_width, f32 sensor_height, f32 focal_length_mm);

Vec3 pixel_dimensions(const Camera *camera, i32 width, i32 height);

#endif // CAMERA_H
