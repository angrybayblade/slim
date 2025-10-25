// Press a key
#include <X11/Xlib.h>
#include <stdio.h>

#include "../src/keyboard.h"

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <key>\n", argv[0]);
    return 1;
  }

  Display *display = XOpenDisplay(NULL);
  if (display == NULL) {
    fprintf(stderr, "Cannot open display\n");
    return 1;
  }

  KeySym keysym = XStringToKeysym(argv[1]);
  if (keysym == 0) {
    fprintf(stderr, "Cannot find keysym for %s\n", argv[1]);
    return 1;
  }

  press_key(display, argv[1]);
  release_key(display, argv[1]);
  XFlush(display);
  XCloseDisplay(display);
  return 0;
}
