#define _USE_MATH_DEFINES
#include <float.h>
#include <math.h>
#include <omp.h>
#include <stdbool.h>
#include <time.h>

#include "camera.h"
#include "color.h"
#include "geometry.h"
#include "path_tracer.h"
#include "random.h"
#include "scene.h"
#include "util.h"
#include "vec3.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define SAMPLE_COUNT (2048)
#define WIDTH (HEIGHT)
#define HEIGHT (2160 * 2)
#define ASPECT_RATIO ((f32)WIDTH / HEIGHT)
#define INV_RAND_MAX (1.0f / RAND_MAX)

void render(const Scene *scene, Vec3 *xyz_framebuffer, i32 frame, bool do_dof);
void post_process(const Vec3 *xyz, Vec3 *rgb, PixelRgb *pixels, f32 factor);
void save_frame(const PixelRgb *pixels, i32 frame_index);

i32 main(void) {
  puts("Allocating memory...");
  Vec3 *xyz_framebuffer = calloc(WIDTH * HEIGHT, sizeof(Vec3));
  Vec3 *rgb_framebuffer = malloc(WIDTH * HEIGHT * sizeof(Vec3));
  PixelRgb *pixels = calloc(WIDTH * HEIGHT, sizeof(PixelRgb));
  if (!xyz_framebuffer || !rgb_framebuffer || !pixels)
    return 1;
  puts("Memory allocated.");

  Scene scene = default_scene();

  const i32 frame_count = 1;
  const f32 total_shift = 2;
  const f32 shift_per_frame = total_shift / frame_count;

  puts("Rendering...");

  clock_t before = clock();

  for (i32 frame = 0; frame < frame_count; ++frame) {
    memset(rgb_framebuffer, 0, WIDTH * HEIGHT * sizeof(Vec3));
    memset(pixels, 0, WIDTH * HEIGHT * sizeof(PixelRgb));

    render(&scene, xyz_framebuffer, frame, false);

    // post-processing
    f32 factor = WAVELENGTH_RANGE / illuminant_N(scene.light_color);
    post_process(xyz_framebuffer, rgb_framebuffer, pixels, factor);

    save_frame(pixels, frame);

    scene.spheres[16].center.x += shift_per_frame;
  }

  clock_t after = clock();

  puts("Render done.");

  f32 elapsed = (f32)(after - before) / CLOCKS_PER_SEC;
  printf("\n%f s\n", elapsed);

  free(rgb_framebuffer);
  free(pixels);
  free(xyz_framebuffer);

  return 0;
}

void render(const Scene *scene, Vec3 *xyz_framebuffer, i32 frame, bool do_dof) {
  memset(xyz_framebuffer, 0, WIDTH * HEIGHT * sizeof(Vec3));
  Vec3 pix_dim = pixel_dimensions(scene->camera, WIDTH, HEIGHT);
  Vec3 half_pix_dim = vec3_muls(pix_dim, 0.5);
  f32 pupil_radius = entrance_pupil_radius(scene->camera);
  f32 pupil_radius_sq = pupil_radius * pupil_radius;
  f32 f = scene->camera->focal_distance / scene->camera->focal_length;
  f32 bucket_width = (f32)WAVELENGTH_RANGE / SAMPLE_COUNT;
  i32 rows_done = 0;
#pragma omp parallel for schedule(dynamic, 1)
  for (i32 row = 0; row < HEIGHT; ++row) {
    Vec3 pixel_position = {.z = -scene->camera->focal_length};
    i32 row_offset = WIDTH * row;
    pixel_position.y = HEIGHT / 2 - row - 0.5f;
    for (i32 col = 0; col < WIDTH; ++col) {
      i32 idx = row_offset + col;
      pixel_position.x = col - WIDTH / 2 + 0.5f;
      Vec3 scaled_pixel_position = vec3_mul(pixel_position, pix_dim);
      f32 bucket_start = MIN_WAVELENGTH;
      Vec3 xyz_acc = {0};
      for (i32 sample = 0; sample < SAMPLE_COUNT; ++sample) {
        Vec3 shift = {0};
        if (do_dof) {
          do {
            shift.x = randf(-pupil_radius, pupil_radius);
            shift.y = randf(-pupil_radius, pupil_radius);
          } while (vec3_dot(shift, shift) > pupil_radius_sq);
        }
        Vec3 origin = vec3_add(scene->camera->position, shift);
        Vec3 jittered_pixel_position = scaled_pixel_position;
        jittered_pixel_position.x += randf(-half_pix_dim.x, half_pix_dim.x);
        jittered_pixel_position.y += randf(-half_pix_dim.y, half_pix_dim.y);
        jittered_pixel_position = vec3_muls(jittered_pixel_position, f);
        Vec3 raw_ray_dir = vec3_sub(jittered_pixel_position, origin);
        Vec3 ray_dir = vec3_normalize(raw_ray_dir);
        Ray3 ray = {.origin = origin, .direction = ray_dir};
        f32 lambda = bucket_start + randf(0, bucket_width);
        RayTracingArgs args = {.ray = ray, .depth = 0, .lambda = lambda};
        f32 intensity = trace_ray(scene, &args);
        Vec3 xyz = vec3_muls(wavelength_to_xyz(lambda), intensity);
        xyz_acc = vec3_add(xyz_acc, xyz);
        bucket_start += bucket_width;
      }
      xyz_framebuffer[idx] = xyz_acc;
    }
    i32 progress;
#pragma omp atomic capture
    {
      rows_done++;
      progress = rows_done;
    }

    if (progress % (HEIGHT / 100) == 0 || progress == HEIGHT) {
#pragma omp critical
      {
        printf("%6.2f%% of frame %d\r", 100.0 * progress / HEIGHT, frame);
        fflush(stdout);
      }
    }
  }
}

void post_process(const Vec3 *xyz, Vec3 *rgb, PixelRgb *pixels, f32 factor) {
  for (i32 i = 0; i < WIDTH * HEIGHT; ++i) {
    Vec3 xyz_tmp = vec3_divs(xyz[i], (f32)SAMPLE_COUNT);
    xyz_tmp = vec3_muls(xyz_tmp, factor);
    rgb[i] = vec3_maxs(xyz_to_rgb(xyz_tmp), 0);

    // tone mapping
    f32 a = 2.51f;
    f32 b = 0.03f;
    f32 c = 2.43f;
    f32 d = 0.59f;
    f32 e = 0.14f;
    Vec3 a_rgb = vec3_muls(rgb[i], a);
    Vec3 ab_rgb = vec3_adds(a_rgb, b);
    Vec3 ab_rgb2 = vec3_mul(ab_rgb, rgb[i]);
    Vec3 c_rgb = vec3_muls(rgb[i], c);
    Vec3 cd_rgb = vec3_adds(c_rgb, d);
    Vec3 cd_rgb2 = vec3_mul(cd_rgb, rgb[i]);
    Vec3 cde_rgb2 = vec3_adds(cd_rgb2, e);
    Vec3 mapped = vec3_div(ab_rgb2, cde_rgb2);
    Vec3 clamped = vec3_clamp(mapped, 0, 1);

    // gamma correction
    Vec3 corrected = vec3_pows(clamped, 1 / 2.2);

    // dithering
    Vec3 scaled = vec3_muls(corrected, 255);
    Vec3 dithered = vec3_round(vec3_adds(scaled, randf(-0.5, 0.5)));
    clamped = vec3_clamp(dithered, 0, 255);
    pixels[i] = (PixelRgb){(u8)clamped.x, (u8)clamped.y, (u8)clamped.z};
  }
}

void save_frame(const PixelRgb *pixels, i32 frame_index) {
  char filename[256] = {0};
  sprintf(filename, "anim/%d.png", frame_index);
  stbi_write_png(filename, WIDTH, HEIGHT, 3, pixels, 0);
}
