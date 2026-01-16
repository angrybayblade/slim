#ifndef SLIM_SERIAL_H_
#define SLIM_SERIAL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <poll.h>

// Linux headers
#include <errno.h>   // Error integer and strerror() function
#include <fcntl.h>   // Contains file controls like O_RDWR
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h>  // write(), read(), close()

// Display headers
#include <X11/Xlib.h>

// Keyboard headers
#include "./keyboard.h"

#define BUF_SIZE 2        // {INT}{NEWLINE}
#define SLEEP_TIME 100000 // 100ms
#define POLL_TIMEOUT 100  // 100ms

typedef struct {
  int partial;
  char *value;
  KeyType type;
} macro_t;

typedef struct {
  int size;
  char *str;
  macro_t *sequence;
} macro_sequence_t;

int open_serial_port(char *serial_port_name);

int configure_serial_port(int serial_port);

void log_message(int serial_fd);

int setup_loop(int serial_fd);

int serial_loop(int serial_fd, macro_sequence_t *macros, int n_macros);

void handle_button(int button, macro_sequence_t *macros, int n_macros);

#endif // SLIM_SERIAL_H_
