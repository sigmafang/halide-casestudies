static const char* bilateral_filter_fast_simple_cl_source =
"/* This file is part of GEGL                                                  \n"
" *                                                                            \n"
" * GEGL is free software; you can redistribute it and/or                      \n"
" * modify it under the terms of the GNU Lesser General Public                 \n"
" * License as published by the Free Software Foundation; either               \n"
" * version 3 of the License, or (at your option) any later version.           \n"
" *                                                                            \n"
" * GEGL is distributed in the hope that it will be useful,                    \n"
" * but WITHOUT ANY WARRANTY; without even the implied warranty of             \n"
" * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU          \n"
" * Lesser General Public License for more details.                            \n"
" *                                                                            \n"
" * You should have received a copy of the GNU Lesser General Public           \n"
" * License along with GEGL; if not, see <http://www.gnu.org/licenses/>.       \n"
" *                                                                            \n"
" * Copyright 2013 Victor Oliveira (victormatheus@gmail.com)                   \n"
" */                                                                           \n"
"                                                                              \n"
"#define GRID(x,y,z) grid[x+sw*(y + z * sh)]                                   \n"
"                                                                              \n"
"__kernel void bilateral_init(__global float8 *grid,                           \n"
"                             int sw,                                          \n"
"                             int sh,                                          \n"
"                             int depth)                                       \n"
"{                                                                             \n"
"  const int gid_x = get_global_id(0);                                         \n"
"  const int gid_y = get_global_id(1);                                         \n"
"                                                                              \n"
"  for (int d=0; d<depth; d++)                                                 \n"
"    {                                                                         \n"
"      GRID(gid_x,gid_y,d) = (float8)(0.0f);                                   \n"
"    }                                                                         \n"
"}                                                                             \n"
"                                                                              \n"
"__kernel void bilateral_downsample(__global const float4 *input,              \n"
"                                   __global       float2 *grid,               \n"
"                                   int width,                                 \n"
"                                   int height,                                \n"
"                                   int sw,                                    \n"
"                                   int sh,                                    \n"
"                                   int   s_sigma,                             \n"
"                                   float r_sigma)                             \n"
"{                                                                             \n"
"  const int gid_x = get_global_id(0);                                         \n"
"  const int gid_y = get_global_id(1);                                         \n"
"                                                                              \n"
"  for (int ry=0; ry < s_sigma; ry++)                                          \n"
"    for (int rx=0; rx < s_sigma; rx++)                                        \n"
"      {                                                                       \n"
"        const int x = clamp(gid_x * s_sigma - s_sigma/2 + rx, 0, width -1);   \n"
"        const int y = clamp(gid_y * s_sigma - s_sigma/2 + ry, 0, height-1);   \n"
"                                                                              \n"
"        const float4 val = input[y * width + x];                              \n"
"                                                                              \n"
"        const int4 z = convert_int4(val * (1.0f/r_sigma) + 0.5f);             \n"
"                                                                              \n"
"        grid[4*(gid_x+sw*(gid_y + z.x * sh))+0] += (float2)(val.x, 1.0f);     \n"
"        grid[4*(gid_x+sw*(gid_y + z.y * sh))+1] += (float2)(val.y, 1.0f);     \n"
"        grid[4*(gid_x+sw*(gid_y + z.z * sh))+2] += (float2)(val.z, 1.0f);     \n"
"        grid[4*(gid_x+sw*(gid_y + z.w * sh))+3] += (float2)(val.w, 1.0f);     \n"
"                                                                              \n"
"        barrier (CLK_GLOBAL_MEM_FENCE);                                       \n"
"      }                                                                       \n"
"}                                                                             \n"
"                                                                              \n"
"__kernel void bilateral_blur_x(__global const float8 *grid,                   \n"
"                               __global       float8 *blurx,                  \n"
"                               int sw,                                        \n"
"                               int sh,                                        \n"
"                               int depth)                                     \n"
"{                                                                             \n"
"  const int x = get_global_id(0);                                             \n"
"  const int y = get_global_id(1);                                             \n"
"                                                                              \n"
"  for (int d=0; d<depth; d++)                                                 \n"
"    {                                                                         \n"
"      const int xp = max(x - 1, 0);                                           \n"
"      const int xn = min(x + 1, sw-1);                                        \n"
"                                                                              \n"
"      float8 v =        grid[xp+sw*(y + d * sh)] +                            \n"
"                 4.0f * grid[x +sw*(y + d * sh)] +                            \n"
"                        grid[xn+sw*(y + d * sh)];                             \n"
"                                                                              \n"
"      blurx[x+sw*(y + d * sh)] = v;                                           \n"
"    }                                                                         \n"
"}                                                                             \n"
"                                                                              \n"
"__kernel void bilateral_blur_y(__global const float8 *blurx,                  \n"
"                               __global       float8 *blury,                  \n"
"                               int sw,                                        \n"
"                               int sh,                                        \n"
"                               int depth)                                     \n"
"{                                                                             \n"
"  const int x = get_global_id(0);                                             \n"
"  const int y = get_global_id(1);                                             \n"
"                                                                              \n"
"  for (int d=0; d<depth; d++)                                                 \n"
"    {                                                                         \n"
"      const int yp = max(y - 1, 0);                                           \n"
"      const int yn = min(y + 1, sh-1);                                        \n"
"                                                                              \n"
"      float8 v =        blurx[x+sw*(yp + d * sh)] +                           \n"
"                 4.0f * blurx[x+sw*(y  + d * sh)] +                           \n"
"                        blurx[x+sw*(yn + d * sh)];                            \n"
"                                                                              \n"
"      blury[x+sw*(y + d * sh)] = v;                                           \n"
"    }                                                                         \n"
"}                                                                             \n"
"                                                                              \n"
"__kernel void bilateral_blur_z(__global const float8 *blury,                  \n"
"                               __global       float2 *blurz_r,                \n"
"                               __global       float2 *blurz_g,                \n"
"                               __global       float2 *blurz_b,                \n"
"                               __global       float2 *blurz_a,                \n"
"                               int sw,                                        \n"
"                               int sh,                                        \n"
"                               int depth)                                     \n"
"{                                                                             \n"
"  const int x = get_global_id(0);                                             \n"
"  const int y = get_global_id(1);                                             \n"
"                                                                              \n"
"  for (int d=0; d<depth; d++)                                                 \n"
"    {                                                                         \n"
"      const int dp = max(d - 1, 0);                                           \n"
"      const int dn = min(d + 1, depth-1);                                     \n"
"                                                                              \n"
"      float8 v =        blury[x+sw*(y + dp * sh)] +                           \n"
"                 4.0f * blury[x+sw*(y + d  * sh)] +                           \n"
"                        blury[x+sw*(y + dn * sh)];                            \n"
"                                                                              \n"
"      blurz_r[x+sw*(y + d * sh)] = v.s01;                                     \n"
"      blurz_g[x+sw*(y + d * sh)] = v.s23;                                     \n"
"      blurz_b[x+sw*(y + d * sh)] = v.s45;                                     \n"
"      blurz_a[x+sw*(y + d * sh)] = v.s67;                                     \n"
"    }                                                                         \n"
"}                                                                             \n"
"                                                                              \n"
"__kernel void bilateral_interpolate(__global    const float4  *input,         \n"
"                                    __global    const float2  *blurz_r,       \n"
"                                    __global    const float2  *blurz_g,       \n"
"                                    __global    const float2  *blurz_b,       \n"
"                                    __global    const float2  *blurz_a,       \n"
"                                    __global          float4  *smoothed,      \n"
"                                    int   width,                              \n"
"                                    int   sw,                                 \n"
"                                    int   sh,                                 \n"
"                                    int   depth,                              \n"
"                                    int   s_sigma,                            \n"
"                                    float r_sigma)                            \n"
"{                                                                             \n"
"  const int x = get_global_id(0);                                             \n"
"  const int y = get_global_id(1);                                             \n"
"                                                                              \n"
"  const float  xf = (float)(x) / s_sigma;                                     \n"
"  const float  yf = (float)(y) / s_sigma;                                     \n"
"  const float4 zf = input[y * width + x] / r_sigma;                           \n"
"                                                                              \n"
"  float8 val;                                                                 \n"
"                                                                              \n"
"  int  x1 = (int)xf;                                                          \n"
"  int  y1 = (int)yf;                                                          \n"
"  int4 z1 = convert_int4(zf);                                                 \n"
"                                                                              \n"
"  int  x2 = min(x1+1, sw-1);                                                  \n"
"  int  y2 = min(y1+1, sh-1);                                                  \n"
"  int4 z2 = min(z1+1, depth-1);                                               \n"
"                                                                              \n"
"  float  x_alpha = xf - x1;                                                   \n"
"  float  y_alpha = yf - y1;                                                   \n"
"  float4 z_alpha = zf - floor(zf);                                            \n"
"                                                                              \n"
"  #define BLURZ_R(x,y,z) blurz_r[x+sw*(y+z*sh)]                               \n"
"  #define BLURZ_G(x,y,z) blurz_g[x+sw*(y+z*sh)]                               \n"
"  #define BLURZ_B(x,y,z) blurz_b[x+sw*(y+z*sh)]                               \n"
"  #define BLURZ_A(x,y,z) blurz_a[x+sw*(y+z*sh)]                               \n"
"                                                                              \n"
"  val.s04 = mix(mix(mix(BLURZ_R(x1, y1, z1.x), BLURZ_R(x2, y1, z1.x), x_alpha),\n"
"                    mix(BLURZ_R(x1, y2, z1.x), BLURZ_R(x2, y2, z1.x), x_alpha), y_alpha),\n"
"                mix(mix(BLURZ_R(x1, y1, z2.x), BLURZ_R(x2, y1, z2.x), x_alpha),\n"
"                    mix(BLURZ_R(x1, y2, z2.x), BLURZ_R(x2, y2, z2.x), x_alpha), y_alpha), z_alpha.x);\n"
"                                                                              \n"
"  val.s15 = mix(mix(mix(BLURZ_G(x1, y1, z1.y), BLURZ_G(x2, y1, z1.y), x_alpha),\n"
"                    mix(BLURZ_G(x1, y2, z1.y), BLURZ_G(x2, y2, z1.y), x_alpha), y_alpha),\n"
"                mix(mix(BLURZ_G(x1, y1, z2.y), BLURZ_G(x2, y1, z2.y), x_alpha),\n"
"                    mix(BLURZ_G(x1, y2, z2.y), BLURZ_G(x2, y2, z2.y), x_alpha), y_alpha), z_alpha.y);\n"
"                                                                              \n"
"  val.s26 = mix(mix(mix(BLURZ_B(x1, y1, z1.z), BLURZ_B(x2, y1, z1.z), x_alpha),\n"
"                    mix(BLURZ_B(x1, y2, z1.z), BLURZ_B(x2, y2, z1.z), x_alpha), y_alpha),\n"
"                mix(mix(BLURZ_B(x1, y1, z2.z), BLURZ_B(x2, y1, z2.z), x_alpha),\n"
"                    mix(BLURZ_B(x1, y2, z2.z), BLURZ_B(x2, y2, z2.z), x_alpha), y_alpha), z_alpha.z);\n"
"                                                                              \n"
"  val.s37 = mix(mix(mix(BLURZ_A(x1, y1, z1.w), BLURZ_A(x2, y1, z1.w), x_alpha),\n"
"                    mix(BLURZ_A(x1, y2, z1.w), BLURZ_A(x2, y2, z1.w), x_alpha), y_alpha),\n"
"                mix(mix(BLURZ_A(x1, y1, z2.w), BLURZ_A(x2, y1, z2.w), x_alpha),\n"
"                    mix(BLURZ_A(x1, y2, z2.w), BLURZ_A(x2, y2, z2.w), x_alpha), y_alpha), z_alpha.w);\n"
"                                                                              \n"
"  smoothed[y * width + x] = val.s0123/val.s4567;                              \n"
"}                                                                             \n"
;
