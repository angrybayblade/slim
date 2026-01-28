// C library headers
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

// Display headers
#include <X11/Xlib.h>

// Slim headers
#include "./keyboard.h"
#include "./serial.h"

#define CMD_START "S\n";
#define CMD_CONFIG "C\n";

#define SER_CMD_LEN 1
#define SER_SIZE_HEADER_LEN 2

#define SER_F_LOG '0' // log message
#define SER_F_STP '1' // setup requirement
#define SER_F_BTN '2' // button press

#ifndef SLIM_DEBUG
#define SLIM_DEBUG 0
#endif

#define if_dbg if (SLIM_DEBUG)

int serial_fd;

void util_handle_shutdown_signal(int sig) {
  close(serial_fd);
  return;
}

/**
 * Open the serial port
 * @return The serial port file descriptor
 */
int open_serial_port(char *serial_port_name) {
  return open(serial_port_name, O_RDWR | O_NOCTTY | O_NDELAY);
}

/**
 * Configure the serial port
 * @param serial_port The serial port file descriptor
 * @return EXIT_SUCCESS if successful, EXIT_FAILURE otherwise
 */
int configure_serial_port(int serial_port) {
  // Configure the serial port
  struct termios options;
  if (tcgetattr(serial_port, &options) != 0) {
    printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
    return EXIT_FAILURE;
  }

  /* SEt Baud Rate */
  cfsetispeed(&options, B9600);
  cfsetospeed(&options, B9600);

  // I don't know what this is exactly
  options.c_cflag |= (CLOCAL | CREAD);

  // Set the Charactor size
  options.c_cflag &= ~CSIZE; /* Mask the character size bits */
  options.c_cflag |= CS8;    /* Select 8 data bits */

  // Set parity - No Parity (8N1)
  options.c_cflag &= ~PARENB;
  options.c_cflag &= ~CSTOPB;
  options.c_cflag &= ~CSIZE;
  options.c_cflag |= CS8;

  // Enable Raw Input
  options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

  // Disable Software Flow control
  options.c_iflag &= ~(IXON | IXOFF | IXANY);

  // Chose raw (not processed) output
  options.c_oflag &= ~OPOST;
  if (tcsetattr(serial_port, TCSANOW, &options) == -1) {
    printf("Error with tcsetattr = %s\n", strerror(errno));
    return EXIT_FAILURE;
  }

  // uncomment when not using poll
  // Set the serial port to non-blocking
  // if (fcntl(serial_port, F_SETFL, FNDELAY) == -1) {
  //   printf("Error with fcntl = %s\n", strerror(errno));
  //   return EXIT_FAILURE;
  // }
  return EXIT_SUCCESS;
}

/**
 * Handle the button
 * @param button The button number
 */
void handle_button(int button, macro_sequence_t *macros, int n_macros) {
  if (button < 0 || button >= n_macros) {
    fprintf(stderr, "[err] Invalid button number: %d\n", button);
    return;
  }

  Display *display = XOpenDisplay(NULL);
  if (display == NULL) {
    fprintf(stderr, "[err] XOpenDisplay = %s\n", strerror(errno));
    return;
  }

  int n_partial = 0;
  char **partial = malloc(sizeof(char *));

  macro_sequence_t *macro_seq = &macros[button];
  if_dbg printf("[dbg] %s\n", macro_seq->str);
  for (int i = 0; i < macro_seq->size; i++) {
    macro_t *macro = &macros[button].sequence[i];
    if (macro->type == KEY_PHRASE) {
      send_phrase(display, macro->value);
      continue;
    }

    if (!macro->partial) {
      send_special_key(display, macro->value);
      continue;
    }

    int idx = n_partial++;
    partial = realloc(partial, n_partial * sizeof(char *));
    partial[idx] = macro->value;

    // partially send key
    press_key(display, macro->value);
  }

  for (int i = n_partial; i > 0; i--)
    release_key(display, partial[i - 1]);
  XCloseDisplay(display);
}

/**
 * Signal loop
 * @param serial_port The serial port file descriptor
 * @return EXIT_SUCCESS if successful, EXIT_FAILURE otherwise
 */
int serial_loop(int serial_fd, macro_sequence_t *macros, int n_macros) {
  int setup_complete = 0;
  int poll_ret;
  int msg_size;
  int byt_size; // The number of bytes read

  char h_cmd[SER_CMD_LEN];          // The buffer to store the data
  char h_size[SER_SIZE_HEADER_LEN]; // The buffer to store the size header

  // Set up the pollfd struct
  struct pollfd pfds[1] = {0};

  // Set up the pollfd struct
  pfds[0].fd = serial_fd;
  pfds[0].events = POLLIN;

  // Read from the serial port until the program is terminated
  while (1) {
    poll_ret = poll(pfds, 1, POLL_TIMEOUT);
    if (poll_ret < 0) {
      printf("error polling: %s\n", strerror(errno));
      return EXIT_FAILURE;
    }

    // If no data is available, sleep for a short time
    if (poll_ret == 0)
      continue;

    // Read the data from the serial port
    byt_size = read(serial_fd, h_cmd, SER_CMD_LEN); // Read the first character
    if (byt_size == -1)
      continue;

    byt_size =
        read(serial_fd, h_size, SER_SIZE_HEADER_LEN); // Read the size header
    if (byt_size == -1)
      continue;

    msg_size = atoi(h_size);
    if (msg_size == 0)
      continue;

    char *line = malloc(msg_size * sizeof(char));
    byt_size = read(serial_fd, line, msg_size + 1); // Read the line
    if (byt_size == -1)
      continue;

    line[msg_size] = '\0';
    if (!setup_complete) {
      if (strcmp(line, "finished setup") != 0)
        continue;
      setup_complete = 1;
    }

    if_dbg printf("[dbg] cmd: %c size: %d line: %s\n", h_cmd[0], msg_size,
                  line);

    switch (h_cmd[0]) {
    case SER_F_LOG:
      printf("[log] %s\n", line);
      break;
    case SER_F_STP:
      printf("[stp] %s\n", line);
      break;
    case SER_F_BTN:
      printf("[btn] %s\n", line);
      handle_button(atoi(line), macros, n_macros);
      break;
    default:
      fprintf(stderr, "[err] unknown command: %s\n", h_cmd);
      break;
    }
  }

  close(serial_fd);
  return EXIT_SUCCESS;
}

macro_sequence_t *parse_macro(char *macro_str) {
  macro_sequence_t *macro = malloc(sizeof(macro_sequence_t));
  macro->sequence = malloc(sizeof(macro_sequence_t *));
  macro->str = macro_str;

  int iter = 0;
  int count = 0;
  int s_start = 0;
  int s_size = 0;
  char c;

  while ((c = *(macro_str++)) && ++iter) {
    if ((c != '|' || *macro_str != '>') && *macro_str)
      continue;

    int idx = count++;
    macro->sequence =
        realloc(macro->sequence, count * sizeof(macro_sequence_t));

    s_size = iter - s_start;
    s_start = iter + 1;
    if (*(macro_str - s_size) == '^') {
      s_size--;
      macro->sequence[idx].partial = 1;
    }

    macro->sequence[idx].value = calloc(s_size, sizeof(char));
    memcpy(macro->sequence[idx].value, macro_str - s_size,
           *macro_str ? s_size - 1 : s_size);
    macro->sequence[idx].type = get_key_type(macro->sequence[idx].value);
  }

  macro->size = count;
  return macro;
}

macro_sequence_t *parse_macros(int argc, char *argv[]) {
  int n_macros = argc - 2;
  macro_sequence_t *macros = calloc(n_macros, sizeof(macro_sequence_t *));
  for (int i = 0; i < n_macros; i++)
    macros[i] = *parse_macro(argv[i + 2]);
  return macros;
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    fprintf(stderr, "Usage: %s <serial_port_name> ...<cmd>\n", argv[0]);
    return EXIT_FAILURE;
  }

  // Open the serial port
  serial_fd = open_serial_port(argv[1]);
  if (serial_fd == EXIT_FAILURE) {
    fprintf(stderr, "error: opening device %s\n", argv[1]);
    return EXIT_FAILURE;
  }

  // Configure the serial port
  if (configure_serial_port(serial_fd) == EXIT_FAILURE) {
    fprintf(stderr, "error: configuring device %s\n", argv[1]);
    return EXIT_FAILURE;
  }

  // parse macros
  macro_sequence_t *macros = parse_macros(argc, argv);

  // Start the signal loop
  return serial_loop(serial_fd, macros, argc - 2);
}
