#include "keyboard.h"
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <string.h>

struct t_character characters[] = {
    // Uppercase Letters
    {'A', XK_A, 1},
    {'B', XK_B, 1},
    {'C', XK_C, 1},
    {'D', XK_D, 1},
    {'E', XK_E, 1},
    {'F', XK_F, 1},
    {'G', XK_G, 1},
    {'H', XK_H, 1},
    {'I', XK_I, 1},
    {'J', XK_J, 1},
    {'K', XK_K, 1},
    {'L', XK_L, 1},
    {'M', XK_M, 1},
    {'N', XK_N, 1},
    {'O', XK_O, 1},
    {'P', XK_P, 1},
    {'Q', XK_Q, 1},
    {'R', XK_R, 1},
    {'S', XK_S, 1},
    {'T', XK_T, 1},
    {'U', XK_U, 1},
    {'V', XK_V, 1},
    {'W', XK_W, 1},
    {'X', XK_X, 1},
    {'Y', XK_Y, 1},
    {'Z', XK_Z, 1},
    // Lowercase Letters
    {'a', XK_a},
    {'b', XK_b},
    {'c', XK_c},
    {'d', XK_d},
    {'e', XK_e},
    {'f', XK_f},
    {'g', XK_g},
    {'h', XK_h},
    {'i', XK_i},
    {'j', XK_j},
    {'k', XK_k},
    {'l', XK_l},
    {'m', XK_m},
    {'n', XK_n},
    {'o', XK_o},
    {'p', XK_p},
    {'q', XK_q},
    {'r', XK_r},
    {'s', XK_s},
    {'t', XK_t},
    {'u', XK_u},
    {'v', XK_v},
    {'w', XK_w},
    {'x', XK_x},
    {'y', XK_y},
    {'z', XK_z},
    // Space
    {' ', XK_space},
    // Punctuation
    {'!', XK_exclam, 1},
    {'@', XK_at, 1},
    {'#', XK_numbersign, 1},
    {'$', XK_dollar, 1},
    {'%', XK_percent, 1},
    {'^', XK_asciicircum, 1},
    {'&', XK_ampersand, 1},
    {'*', XK_asterisk, 1},
    {'(', XK_parenleft, 1},
    {')', XK_parenright, 1},
    {'-', XK_minus},
    {'_', XK_underscore, 1},
    {'=', XK_equal},
    {'+', XK_plus, 1},
    {'[', XK_bracketleft},
    {']', XK_bracketright},
    {'{', XK_braceleft, 1},
    {'}', XK_braceright, 1},
    {'|', XK_bar, 1},
    {'\\', XK_backslash},
    {';', XK_semicolon},
    {':', XK_colon, 1},
    {'\'', XK_apostrophe},
    {'"', XK_quotedbl, 1},
    {'<', XK_less, 1},
    {'>', XK_greater, 1},
    {',', XK_comma},
    {'.', XK_period},
    {'/', XK_slash},
    {'?', XK_question, 1},
    // Numbers
    {'0', XK_0},
    {'1', XK_1},
    {'2', XK_2},
    {'3', XK_3},
    {'4', XK_4},
    {'5', XK_5},
    {'6', XK_6},
    {'7', XK_7},
    {'8', XK_8},
    {'9', XK_9},
    // Special Keys
};

// Find the keycode for a given key
struct t_character find_keycode_map(Display *display, char key) {
  for (int i = 0; i < sizeof(characters) / sizeof(characters[0]); i++) {
    if (characters[i].key == key) {
      return characters[i];
    }
  }
  return (struct t_character){0, 0, 0};
}

// Press a key
void press_key(Display *display, char *name) {
  KeySym keysym = XStringToKeysym(name);
  if (keysym == 0) {
    fprintf(stderr, "Cannot find keysym for %s\n", name);
    return;
  }

  unsigned int keycode = XKeysymToKeycode(display, keysym);
  XTestFakeKeyEvent(display, keycode, True, CurrentTime);
}

// Release a key
void release_key(Display *display, char *name) {
  KeySym keysym = XStringToKeysym(name);
  if (keysym == 0) {
    fprintf(stderr, "Cannot find keysym for %s\n", name);
    return;
  }

  unsigned int keycode = XKeysymToKeycode(display, keysym);
  XTestFakeKeyEvent(display, keycode, False, CurrentTime);
}

// Send key
void send_key(Display *display, KeySym keysym) {
  unsigned int keycode = XKeysymToKeycode(display, keysym);
  XTestFakeKeyEvent(display, keycode, True, CurrentTime);
  XTestFakeKeyEvent(display, keycode, False, CurrentTime);
}

// Send an enter key to the display
void send_enter(Display *display) {
  send_key(display, XK_Return);
  XFlush(display);
}

// Send a single key to the display
void send_character(Display *display, char key) {
  struct t_character keycode_map = find_keycode_map(display, key);
  if (keycode_map.keysym == 0) {
    fprintf(stderr, "Cannot find keycode_map for %c\n", key);
    return;
  }

  // Press the shift key
  if (keycode_map.requires_shift) {
    press_key(display, SK_SHIFT_L);
  }

  // Send the key
  send_key(display, keycode_map.keysym);

  // Release the shift key
  if (keycode_map.requires_shift) {
    release_key(display, SK_SHIFT_L);
  }

  // Flush the display
  XFlush(display);
}

// Send a special key to the display
void send_special_key(Display *display, char *name) {
  KeySym keysym = XStringToKeysym(name);
  if (keysym == 0) {
    fprintf(stderr, "Cannot find keysym for %s\n", name);
    return;
  }
  send_key(display, keysym);
  XFlush(display);
}

// Send a phrase to the display
void send_phrase(Display *display, char *phrase) {
  for (int i = 0; i < strlen(phrase); i++) {
    send_character(display, phrase[i]);
  }
}
