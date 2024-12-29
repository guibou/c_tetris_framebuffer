#include <fcntl.h>
#include <linux/fb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "graphical.h"

struct fb open_fb(const char* device)
{
  // Open the fb
  int fd = open(device, O_RDWR);
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
  
  // Close the fd, it is no longer required.
  close(fd);

  return fb;
}

void close_fb(struct fb *fb)
{
  munmap(fb->pixels, 0);
}

