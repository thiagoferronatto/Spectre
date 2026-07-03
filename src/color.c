#include "color.h"

Vec3 xyz_to_rgb(Vec3 xyz) {
  f32 r = 2.36461385 * xyz.x - 0.89654057 * xyz.y - 0.46807328 * xyz.z;
  f32 g = -0.51516621 * xyz.x + 1.4264081 * xyz.y + 0.0887581 * xyz.z;
  f32 b = 0.0052037 * xyz.x - 0.01440816 * xyz.y + 1.00920446 * xyz.z;
  return (Vec3){r, g, b};
}

Vec3 wavelength_to_rgb(f32 lambda) {
  return xyz_to_rgb(wavelength_to_xyz(lambda));
}

f32 illuminant_N(SPD illuminant) {
  f32 sum = 0, d_lambda = (f32)WAVELENGTH_RANGE / INTEGRATION_STEPS;
  for (i32 i = 0; i < INTEGRATION_STEPS; ++i) {
    f32 lambda =
        WAVELENGTH_RANGE * ((f32)i / (INTEGRATION_STEPS - 1)) + MIN_WAVELENGTH;
    sum += y_bar(lambda) * lambda_response(lambda, illuminant, (Vec3){0});
  }
  return sum * d_lambda;
}
