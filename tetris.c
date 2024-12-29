#include <fcntl.h>
#include <linux/fb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "graphical.h"
#include "pieces.h"

const int quad_side = 40;
const int quad_margin = 1;

void draw_quad(struct fb *fb, struct pixel color, struct vec2 quad, int x,
               int y) {
  const int px = quad.x * (quad_side + quad_margin) + x;
  const int py = quad.y * (quad_side + quad_margin) + y;

  for (int qy = 0; qy < quad_side; qy++) {
    for (int qx = 0; qx < quad_side; qx++) {
      set_pixel(px + qx, py + qy, fb, color);
    }
  }
}

void draw_shape(struct fb *fb, const struct shape *shape, int rotation, int x, int y) {
  struct pixel color = tetrimino_color(shape->kind);

  for (int i = 0; i < 4; i++) {
    draw_quad(fb, color, rotate(shape->grid[i], rotation), x, y);
  }
}

int main() {
  printf("Welcome to tetris\n");

  struct fb fb = open_fb("/dev/fb0");

  {
    // Clean the complete screen
    for (int y = 0; y < fb.resY; y++) {
      for (int x = 0; x < fb.resX; x++) {
        struct pixel p = {0, 0, 0, 255};
        set_pixel(x, y, &fb, p);
      }
    }
  }

  for (int rotation = 0; rotation < 4; rotation++) {
    for (int shape = 0; shape < nbShapes; shape++) {
      draw_shape(&fb, shapes[shape], rotation,
                 150 + shape * (quad_side + quad_margin) * 5,
                 150 + rotation * (quad_side + quad_margin) * 5);
    }
  }

  close_fb(&fb);
}
