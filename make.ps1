mkdir x64/bin -Force | Out-Null
clang `
  src/main.c src/color.c src/geometry.c `
  src/scene.c src/camera.c `
  -o x64/bin/spectre.exe `
  -O3 `
  -march=native `
  -m64 `
  -finline-functions `
  -mllvm -force-vector-width=8 `
  -ftree-vectorize `
  -fuse-ld=lld `
  -flto `
  -funroll-loops `
  -ffast-math `
  -Iinc `
  -Wall -Wextra `
  -fopenmp `
  -Wno-missing-braces
