#include <fcntl.h>
#include <linux/fb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <threads.h>
#include <time.h>
#include <unistd.h>

#include "graphical.h"
#include "pieces.h"
#include "terminal.h"

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
    struct vec2 set_position = translate(
        translate(rotate(game->current_shape->grid[i],
                         (game->current_shape_rotation + dRotation + 4) % 4),
                  game->current_shape_position),
        dPosition);

    if (game->field[set_position.y * game->width + set_position.x].used ||
        set_position.y >= game->height || set_position.x < 0 ||
        set_position.x >= game->width) {
      return false;
    }
  }
  return true;
}

struct handler_data {
  struct draw_info *draw_info;
  struct game *game;
  bool run;
};

// Draw the game, see the different discussions
void draw(struct draw_info *draw_info, struct game *game) {
  // Draw the field
  // This is *NOT* efficient, mostly because we draw ALL the cases, when
  // actually most of them are not changing.
  for (int y = 0; y < game->height; y++)
    for (int x = 0; x < game->width; x++) {
      struct quad_status *status = &game->field[y * game->width + x];

      if (status->used) {
        draw_quad(draw_info, tetrimino_color(status->by), (struct vec2){x, y});
      } else {
        // We also have a flickering issue possible here because we first set to
        // empty all the cases not used by previous shape, and ...
        draw_quad(draw_info, (struct pixel){0, 0, 0, 255}, (struct vec2){x, y});
      }
    }

  // draw the current shape
  // ... when drawing the current shape, we override a possibly set to black
  // value.
  draw_shape(draw_info, game->current_shape, game->current_shape_rotation,
             game->current_shape_position);
}

int handle_events(struct handler_data *arg) {
  while (arg->run) {
    char c = getchar();

    // All of that is unsafe, because we change the value of the current shape,
    // but this can happen in concurrency with the main loop.
    //
    // So in summary, that's not safe at all.
    if (c == 'a' && (game_check(arg->game, (struct vec2){-1, 0}, 0))) {
      arg->game->current_shape_position.x -= 1;
    }
    if (c == 'e' && (game_check(arg->game, (struct vec2){1, 0}, 0))) {
      arg->game->current_shape_position.x += 1;
    }
    if (c == ',' && (game_check(arg->game, (struct vec2){0, 0}, 3))) {
      arg->game->current_shape_rotation =
          (arg->game->current_shape_rotation + 3) % 4;
    }
    if (c == 'o' && (game_check(arg->game, (struct vec2){0, 0}, 1))) {
      arg->game->current_shape_rotation =
          (arg->game->current_shape_rotation + 1) % 4;
    }

    printf("keyboard: %c\n", c);
  }

  return 0;
}

// This function should be tested, I have no idea if it really works ;)
void game_score(struct game *game) {
  int scored = 0;
  // for each lines, we check if they are complete
  int current_line = game->height - 1;
  while ((current_line - scored) >= 0) {
    bool full_line = true;
    for (int x = 0; x < game->width; x++) {
      if (!game->field[(current_line - scored) * game->width + x].used) {
        full_line = false;
        break;
      }
    }

    // The line is full, we score and we move to the next line
    if (full_line) {
      scored += 1;
    } else {
      // The currently evaluated line can be moved down if required
      if (scored > 0) {
        for (int x = 0; x < game->width; x++) {
          game->field[current_line * game->width + x] =
              game->field[(current_line - scored) * game->width + x];
        }
      }

      current_line -= 1;
    }
  }

  // clean the remaining lines
  while (current_line >= 0) {
    for (int x = 0; x < game->width; x++) {
      if (current_line - scored >= 0)
        game->field[current_line * game->width + x] =
            game->field[(current_line - scored) * game->width + x];
      else
        game->field[current_line * game->width + x] =
            (struct quad_status){false, O};
    }

    current_line -= 1;
  }

  if (scored > 0) {
    game->score += scored;
    printf("You scored %d. New score %d.\n", scored, game->score);
  }
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

  // This thread is handling "non-blocking" keyboard input and edit the `game`
  // structure, WITHOUT any concurrency mecanism. In short, it is unsafe and it
  // *MUST* be fixed in order to provide something of quality.
  thrd_t thr;
  struct handler_data handler_data = {&draw_info, &game, true};
  thrd_create(&thr, (thrd_start_t)&handle_events, (void *)&handler_data);

  cm_off();
  while (1) {
    // The draw is done at each loop
    draw(&draw_info, &game);

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

      game_score(&game);
    }

    thrd_sleep(&(struct timespec){.tv_nsec = game.speed * 1000 * 1000}, NULL);
  }
  cm_on();

  printf("End of game. Score: %d.\n", game.score);
  handler_data.run = false;
  thrd_join(thr, NULL);

  free(game.field);
  close_fb(&fb);
}

// A few TODOS:
//
// - fix game logic, rotation, soft/hard drop, implement correct T rotation,
// ... But I don't really care
// - fix the concurrency issues: having this thread running in parallel of the
// main loop in order to gather "keyboard" event is wrong.
// - The refresh rate MUST not be correlated with the game steps.
// - Finally, we can dramatically optimise the drawing logic, by not redrawing
// everything at each loop.
// - Then, they may be room for micro optimisation. The pixels are 32bits, so
// if correctly aligned, each color is just an int32 copy, instead of 4 char.
// We have no guarantee that the compiler will optimise that.
