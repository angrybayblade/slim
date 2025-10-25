#ifndef SLIM_KEYBOARD_H_
#define SLIM_KEYBOARD_H_

#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <string.h>

// General Keys
#define SK_ENTER "Return"
#define SK_BACKSPACE "BackSpace"
#define SK_TAB "Tab"
#define SK_ESC "Escape"
#define SK_CTRL_L "Control_L"
#define SK_CTRL_R "Control_R"
#define SK_ALT_L "Alt_L"
#define SK_ALT_R "Alt_R"
#define SK_SHIFT_L "Shift_L"
#define SK_SHIFT_R "Shift_R"
#define SK_CAPS_LOCK "Caps_Lock"
#define SK_NUM_LOCK "Num_Lock"
#define SK_PRINT_SCREEN "Print"
#define SK_SCROLL_LOCK "Scroll_Lock"
#define SK_DELETE "Delete"
#define SK_INSERT "Insert"
#define SK_PAUSE "Pause"
#define SK_BREAK "Break"
#define SK_SUPER_L "Super_L"
#define SK_SUPER_R "Super_R"

// Navigation Keys
#define SK_HOME "Home"
#define SK_END "End"
#define SK_PAGE_UP "Page_Up"
#define SK_PAGE_DOWN "Page_Down"

// Arrow Keys
#define SK_UP "Up"
#define SK_DOWN "Down"
#define SK_LEFT "Left"
#define SK_RIGHT "Right"

// Function Keys
#define SK_F1 "F1"
#define SK_F2 "F2"
#define SK_F3 "F3"
#define SK_F4 "F4"
#define SK_F5 "F5"
#define SK_F6 "F6"
#define SK_F7 "F7"
#define SK_F8 "F8"
#define SK_F9 "F9"
#define SK_F10 "F10"
#define SK_F11 "F11"
#define SK_F12 "F12"

struct t_character {
  char key;
  KeySym keysym;
  int requires_shift;
};

struct t_special_key {
  char *name;
  KeySym keysym;
};

struct t_character find_keycode_map(Display *display, char key);
void press_key(Display *display, char *name);
void release_key(Display *display, char *name);
void send_key(Display *display, KeySym keysym);
void send_character(Display *display, char key);
void send_special_key(Display *display, char *name);
void send_phrase(Display *display, char *phrase);
void send_enter(Display *display);

#endif // SLIM_KEYBOARD_H_