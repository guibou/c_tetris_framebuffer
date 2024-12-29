#include <fcntl.h>
#include <linux/fb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "graphical.h"
#include "pieces.h"

#define min(a, b) (((a) < (b)) ? (a) : (b))

struct draw_info {
  struct fb *fb;
  int quad_side;
  int quad_margin;
  int offset_x;
  int offset_y;
};

void draw_quad(struct draw_info *draw_info, struct pixel color,
               struct vec2 quad, int x, int y) {
  const int px = quad.x * (draw_info->quad_side + draw_info->quad_margin) + x;
  const int py = quad.y * (draw_info->quad_side + draw_info->quad_margin) + y;

  for (int qy = 0; qy < draw_info->quad_side; qy++) {
    for (int qx = 0; qx < draw_info->quad_side; qx++) {
      set_pixel(px + qx, py + qy, draw_info->fb, color);
    }
  }
}

void draw_shape(struct draw_info *draw_info, const struct shape *shape,
                int rotation, int x, int y) {
  struct pixel color = tetrimino_color(shape->kind);

  for (int i = 0; i < 4; i++) {
    draw_quad(draw_info, color, rotate(shape->grid[i], rotation), x, y);
  }
}

// According to https://tetris.fandom.com/wiki/Playfield
const int FIELD_WIDTH = 10;
const int FIELD_HEIGHT = 16;

// Tagged union representing the state of the grid. If "used", then there is a
// color inside it.
struct quad_status {
  bool used;
  enum Tetrimino by;
};

struct game {
  struct quad_status *field;
  int width;
  int height;
};

int main() {
  printf("Welcome to tetris\n");

  // Setup the field
  struct game game;
  game.width = FIELD_WIDTH;
  game.height = FIELD_HEIGHT;
  game.field = malloc(sizeof(struct quad_status) * game.width * game.height);

  for (int y = 0; y < game.height; y++)
    for (int x = 0; x < game.width; x++)
      game.field[y * game.width + x] = (struct quad_status){false, O};

  struct fb fb = open_fb("/dev/fb0");

  // Setup drawing informations
  // We want the biggest drawing area
  //
  // 1px of marging
  //
  // the footprint in one dimension is hence:
  //      dim = nbQuad * quadSize + (nbQuad - 1) * marginSize
  //      ==> quadSize = (dim - (nbQuad - 1) * marginSize) / nbQuad
  struct draw_info draw_info;
  draw_info.quad_margin = 1;
  draw_info.quad_side =
      min((fb.resX - (game.width - 1) * draw_info.quad_margin) / game.width,
          (fb.resY - (game.height - 1) * draw_info.quad_margin) / game.height);

  draw_info.offset_x = (fb.resX - (draw_info.quad_side * game.width +
                                   draw_info.quad_margin * (game.width - 1))) /
                       2;
  draw_info.offset_y = (fb.resY - (draw_info.quad_side * game.height +
                                   draw_info.quad_margin * (game.height - 1))) /
                       2;
  draw_info.fb = &fb;

  // Clean the complete screen once
  for (int y = 0; y < fb.resY; y++) {
    for (int x = 0; x < fb.resX; x++) {
      struct pixel p = {50, 50, 50, 255};
      set_pixel(x, y, &fb, p);
    }
  }

  while (1) {
    // Draw the field
    for (int y = 0; y < game.height; y++)
      for (int x = 0; x < game.width; x++) {
        struct quad_status *status = &game.field[y * game.width + x];

        if (status->used) {
          draw_quad(&draw_info, tetrimino_color(status->by),
                    (struct vec2){x, y}, draw_info.offset_x,
                    draw_info.offset_y);
        } else {
          draw_quad(&draw_info, (struct pixel) {0, 0, 0, 255},
                    (struct vec2){x, y}, draw_info.offset_x,
                    draw_info.offset_y);
        }
      }
    sleep(1);
  }

  close_fb(&fb);
}
