#include <fcntl.h>
#include <linux/fb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

struct pixel {
  char r, g, b, a;
};

struct fb {
  struct pixel *pixels;
  int resX, resY;
};

void set_pixel(int x, int y, struct fb *fb, struct pixel p) {
  struct pixel *pixel = fb->pixels + fb->resX * y + x;
  pixel->r = p.r;
  pixel->g = p.g;
  pixel->b = p.b;
  pixel->a = p.a;
}

int main() {
  printf("Welcome to tetris\n");

  // Open the fb
  int fd = open("/dev/fb0", O_RDWR);
  struct fb fb;
  {

    // Get resolution, read the kernel fb.h documentation for them
    struct fb_var_screeninfo info;
    ioctl(fd, FBIOGET_VSCREENINFO, &info);
    const int resX = info.xres;
    const int resY = info.yres;

    printf("Initialized fb of size %dx%d\n", resX, resY);
    // We'll assume 32 bits per pixel, because I'm lazy.
    struct pixel *pixels =
        mmap(NULL, resX * resY * 4, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    fb.pixels = pixels;
    fb.resX = info.xres;
    fb.resY = info.yres;
  }

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

  munmap(fb.pixels, 0);
  close(fd);
}
