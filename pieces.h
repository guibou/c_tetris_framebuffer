#pragma once

// We are trying to follow the guidelines at
// https://tetris.fandom.com/wiki/Tetris_Guideline

#include "graphical.h"

// https://fr.wikipedia.org/wiki/Tetris#Pi%C3%A8ces_du_jeu
enum Tetrimino { I, O, T, L, J, Z, S };

// Colors according to https://www.rapidtables.com/web/color/index.html
const struct pixel cyan = {0, 255, 255, 0};
const struct pixel yellow = {255, 255, 0, 0};
const struct pixel purple = {128, 0, 128, 0};
const struct pixel red = {255, 0, 0, 0};
const struct pixel green = {0, 255, 0, 0};
const struct pixel blue = {0, 0, 255, 0};
const struct pixel orange = {255, 165, 0, 0};

// According to https://tetris.fandom.com/wiki/Tetris_Guideline
struct pixel tetrimino_color(enum Tetrimino t) {
  switch (t) {
  case I:
    return cyan;
  case O:
    return yellow;
  case T:
    return purple;
  case S:
    return green;
  case Z:
    return red;
  case J:
    return blue;
  case L:
    return orange;
  }

  // Arbitrary, in case the switch is NOT exhaustive
  return blue;
}

struct vec2 {
  int x, y;
};

struct shape {
  enum Tetrimino kind;
  struct vec2 grid[4];
};

const struct shape shape_I = {I, {{-1, 0}, {0, 0}, {1, 0}, {2, 0}}};

const struct shape shape_O = {O, {{0, 0}, {1, 0}, {0, 1}, {1, 1}}};

const struct shape shape_T = {T, {{-1, 0}, {0, 0}, {1, 0}, {0, 1}}};

const struct shape shape_L = {L, {{-1, 0}, {0, 0}, {1, 0}, {-1, 1}}};

const struct shape shape_J = {J, {{-1, 0}, {0, 0}, {1, 0}, {1, 1}}};

const struct shape shape_Z = {Z, {{-1, 0}, {0, 0}, {0, 1}, {1, 1}}};

const struct shape shape_S = {S, {{0, 0}, {1, 0}, {-1, 1}, {0, 1}}};

const struct shape *shapes[] = {&shape_I, &shape_O, &shape_T, &shape_L,
                                &shape_J, &shape_Z, &shape_S};

const int nbShapes = 7;

// Rotate a shape
// It does not follows the tetris guideline about rotation, because I'm too lazy!
struct vec2 rotate(struct vec2 quad, int rotation)
{
    // Functional programming style!
    if(rotation == 0)
        return quad;

    struct vec2 quadRotated = {-quad.y, quad.x};
    return rotate(quadRotated, rotation - 1);
}

