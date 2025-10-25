#include <X11/Xlib.h>
#include <stdio.h>

#include "../src/keyboard.h"

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <phrase>\n", argv[0]);
    return 1;
  }

  Display *display = XOpenDisplay(NULL);
  if (display == NULL) {
    fprintf(stderr, "Cannot open display\n");
    return 1;
  }

  send_phrase(display, argv[1]);
  XFlush(display);
  XCloseDisplay(display);
  return 0;
}
