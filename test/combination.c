// Send sequence of keys
#include <X11/Xlib.h>
#include <stdio.h>

#include "../src/keyboard.h"

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <k1> <k2> <k3> ...\n", argv[0]);
    return 1;
  }

  Display *display = XOpenDisplay(NULL);
  if (display == NULL) {
    fprintf(stderr, "Cannot open display\n");
    return 1;
  }

  for (int i = 1; i < argc; i++) {
    press_key(display, argv[i]);
  }

  for (int i = argc - 1; i >= 1; i--) {
    release_key(display, argv[i]);
  }

  XFlush(display);
  XCloseDisplay(display);
  return 0;
}
