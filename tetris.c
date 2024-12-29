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
  struct vec2 offset;
};

void draw_quad(struct draw_info *draw_info, struct pixel color,
               struct vec2 quad) {
  const int px = quad.x * (draw_info->quad_side + draw_info->quad_margin) +
                 draw_info->offset.x;
  const int py = quad.y * (draw_info->quad_side + draw_info->quad_margin) +
                 draw_info->offset.y;

  for (int qy = 0; qy < draw_info->quad_side; qy++) {
    for (int qx = 0; qx < draw_info->quad_side; qx++) {
      set_pixel(px + qx, py + qy, draw_info->fb, color);
    }
  }
}

void draw_shape(struct draw_info *draw_info, const struct shape *shape,
                int rotation, struct vec2 delta) {
  struct pixel color = tetrimino_color(shape->kind);

  for (int i = 0; i < 4; i++) {
    draw_quad(draw_info, color,
              translate(rotate(shape->grid[i], rotation), delta));
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

  struct vec2 current_shape_position;
  int current_shape_rotation;
  const struct shape *current_shape;
  int score;
  int speed;
};

void game_sample_shape(struct game *game) {
  game->current_shape_rotation = 0;

  // Note: this is a stupid way of generating random number. Don't do that at
  // home, because it is biased, but I don't really care ;)
  game->current_shape = shapes[rand() % nbShapes];
  game->current_shape_position = (struct vec2){game->width / 2, 0};
}

void game_store_shape(struct game *game) {
  for (int i = 0; i < 4; i++) {
    struct vec2 set_position = translate(
        rotate(game->current_shape->grid[i], game->current_shape_rotation),
        game->current_shape_position);

    game->field[set_position.y * game->width + set_position.x] =
        (struct quad_status){true, game->current_shape->kind};
  }
}

bool game_check(struct game *game, struct vec2 dPosition, int dRotation) {
  for (int i = 0; i < 4; i++) {
    struct vec2 set_position =
        translate(translate(rotate(game->current_shape->grid[i],
                                   game->current_shape_rotation + dRotation),
                            game->current_shape_position),
                  dPosition);

    if (game->field[set_position.y * game->width + set_position.x].used ||
        set_position.y >= game->height) {
      return false;
    }
  }
  return true;
}

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

  draw_info.offset =
      (struct vec2){(fb.resX - (draw_info.quad_side * game.width +
                                draw_info.quad_margin * (game.width - 1))) /
                        2,
                    (fb.resY - (draw_info.quad_side * game.height +
                                draw_info.quad_margin * (game.height - 1))) /
                        2};
  draw_info.fb = &fb;

  // Clean the complete screen once
  for (int y = 0; y < fb.resY; y++) {
    for (int x = 0; x < fb.resX; x++) {
      struct pixel p = {50, 50, 50, 255};
      set_pixel(x, y, &fb, p);
    }
  }

  srand(clock());
  game.score = 0;
  game.speed = 250;
  game_sample_shape(&game);

  while (1) {
    // Draw the field
    for (int y = 0; y < game.height; y++)
      for (int x = 0; x < game.width; x++) {
        struct quad_status *status = &game.field[y * game.width + x];

        if (status->used) {
          draw_quad(&draw_info, tetrimino_color(status->by),
                    (struct vec2){x, y});
        } else {
          draw_quad(&draw_info, (struct pixel){0, 0, 0, 255},
                    (struct vec2){x, y});
        }
      }

    // draw the current shape
    draw_shape(&draw_info, game.current_shape, game.current_shape_rotation,
               game.current_shape_position);

    struct timespec tim = {0, game.speed * 1000 * 1000};
    nanosleep(&tim, NULL);

    // Move
    if (game_check(&game, (struct vec2){0, 1}, 0)) {
      game.current_shape_position.y += 1;
    } else {
      game_store_shape(&game);
      game_sample_shape(&game);

      // Check that we are not dead
      if (!game_check(&game, (struct vec2){0, 0}, 0)) {
        break;
      }
    }
  }

  printf("End of game. Score: %d.\n", game.score);

  close_fb(&fb);
}
