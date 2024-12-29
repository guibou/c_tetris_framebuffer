#include <fcntl.h>
#include <linux/fb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "graphical.h"

int main() {
  printf("Welcome to tetris\n");

  struct fb fb = open_fb("/dev/fb0");

  // Bouncing ball animation
  int cX = 0;
  int cY = 0;

  int vX = 1;
  int vY = 1;

  while (1) {
    // Let's draw a circle, green, in the middle
    for (int y = 0; y < fb.resY; y++) {
      for (int x = 0; x < fb.resX; x++) {
        const int radius = 200;
        const int dx = (x - cX);
        const int dy = (y - cY);

        if (dx * dx + dy * dy > radius * radius) {
          struct pixel p = {255, 0, 255, 255};
          set_pixel(x, y, &fb, p);
        } else {
          struct pixel p = {0, 255, 255, 255};
          set_pixel(x, y, &fb, p);
        }
      }
    }

    if (cX < 0 || cX > fb.resX)
      vX = -vX;
    if (cY < 0 || cY > fb.resY)
      vY = -vY;

    cX = cX + vX;
    cY = cY + vY;
  }

  close_fb(&fb);
}
