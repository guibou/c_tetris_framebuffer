#pragma once

#include <assert.h>

struct pixel {
  char r, g, b, a;
};

struct fb {
  struct pixel *pixels;
  int resX, resY;
};

inline void set_pixel(int x, int y, struct fb *fb, struct pixel p) {
  // avoid offscreen drawing
  if (x >= 0 && x < fb->resX && y >= 0 && y < fb->resY) {
    struct pixel *pixel = fb->pixels + fb->resX * y + x;
    pixel->r = p.r;
    pixel->g = p.g;
    pixel->b = p.b;
    pixel->a = p.a;
  }
}

struct fb open_fb(const char *device);
void close_fb(struct fb *fb);
