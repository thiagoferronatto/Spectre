#ifndef COLOR_H
#define COLOR_H

#include "vec3.h"

#define MIN_WAVELENGTH 400
#define MAX_WAVELENGTH 700
#define WAVELENGTH_RANGE (MAX_WAVELENGTH - MIN_WAVELENGTH)
#define INTEGRATION_STEPS 10000

typedef enum SPD {
  SPD_RED = 0,
  SPD_GREEN = 1,
  SPD_BLUE = 2,
  SPD_CYAN = 3,
  SPD_MAGENTA = 4,
  SPD_YELLOW = 5,
  SPD_CHECKERS_XZ = 6,
  SPD_WHITE = 7,
  SPD_BLACK = 8,

  SPD_ILL_E = 100,
  SPD_ILL_FLUORESCENT = 101,
  SPD_ILL_AMBIENT = 102
} SPD;

static inline f32 g(f32 x, f32 mu, f32 tau_1, f32 tau_2) {
  f32 tau = x < mu ? tau_1 : tau_2;
  f32 x_mu = x - mu;
  return expf(-0.5 * tau * tau * x_mu * x_mu);
}

static inline f32 x_bar(f32 lambda) {
  return 1.056 * g(lambda, 599.8, 0.0264, 0.0323) //
         + 0.362 * g(lambda, 442, 0.0624, 0.0374) //
         - 0.065 * g(lambda, 501.1, 0.049, 0.0382);
}

static inline f32 y_bar(f32 lambda) {
  return 0.821 * g(lambda, 568.8, 0.0213, 0.0247) //
         + 0.286 * g(lambda, 530.9, 0.0613, 0.0322);
}

static inline f32 z_bar(f32 lambda) {
  return 1.217 * g(lambda, 437, 0.0845, 0.0278) //
         + 0.681 * g(lambda, 459, 0.0385, 0.0725);
}

static inline Vec3 wavelength_to_xyz(f32 lambda) {
  return (Vec3){x_bar(lambda), y_bar(lambda), z_bar(lambda)};
}

Vec3 xyz_to_rgb(Vec3 xyz);

Vec3 wavelength_to_rgb(f32 lambda);

static inline f32 lambda_response(f32 lambda, SPD spd, Vec3 point) {
  switch (spd) {
  case SPD_RED: {
    f32 lambda_mu = lambda - 650;
    return expf(-0.001 * lambda_mu * lambda_mu);
  }
  case SPD_GREEN: {
    f32 lambda_mu = lambda - 542.5;
    return expf(-0.001 * lambda_mu * lambda_mu);
  }
  case SPD_BLUE: {
    f32 lambda_mu = lambda - 440;
    return expf(-0.001 * lambda_mu * lambda_mu);
  }
  case SPD_CYAN:
    return 0.5 - 0.5 * erff(0.2 * (lambda - 570));
  case SPD_MAGENTA: {
    f32 lambda_mu_0 = lambda - 450, lambda_mu_1 = lambda - 620;
    f32 a = expf(-0.001 * lambda_mu_0 * lambda_mu_0);
    f32 b = expf(-0.001 * lambda_mu_1 * lambda_mu_1);
    return a + b;
  }
  case SPD_YELLOW:
    return 0.5 * erff(0.2 * (lambda - 530)) + 0.5;
  case SPD_CHECKERS_XZ: {
    i32 cx = (i32)floorf(point.x * 5.0f);
    i32 cz = (i32)floorf(point.z * 5.0f);
    return (cx + cz) % 2 ? 1 : 0.01;
  }
  case SPD_BLACK:
    return 0;
  case SPD_WHITE:
  case SPD_ILL_E:
    return 1;
  case SPD_ILL_FLUORESCENT: {
    f32 lambda_mu_0 = lambda - 450, lambda_mu_1 = lambda - 550;
    f32 a = expf(-0.005 * lambda_mu_0 * lambda_mu_0);
    f32 b = expf(-0.0005 * lambda_mu_1 * lambda_mu_1);
    return a + b;
  }
  case SPD_ILL_AMBIENT:
    return 0.005f;
    // return 100.0f / (lambda - 300); // skylight
  }
}

f32 illuminant_N(SPD illuminant);

#endif // COLOR_H
