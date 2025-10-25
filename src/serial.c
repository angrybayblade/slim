// C library headers
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

const char *CMD_START = "S\n";
const char *CMD_CONFIG = "C\n";

// X11 helpers
int open_serial_port(char *serial_port_name);
int configure_serial_port(int serial_port);

// Signal loop helpers
int setup_loop(int serial_port);
int signal_loop(int serial_port, char *phrases[], int n_phrases);
void handle_button(int button, char *phrases[], int n_phrases);

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <serial_port_name>\n", argv[0]);
    return EXIT_FAILURE;
  }

  // Open the serial port
  char *serial_port_name = argv[1];
  int serial_port = open_serial_port(serial_port_name);
  if (serial_port == EXIT_FAILURE) {
    return EXIT_FAILURE;
  }

  // Configure the serial port
  if (configure_serial_port(serial_port) == EXIT_FAILURE) {
    return EXIT_FAILURE;
  }

  // Get the phrases
  int n_phrases = argc - 2;
  char *phrases[] = {};
  for (int i = 2; i < argc; i++) {
    phrases[i - 2] = argv[i];
  }

  // Start the signal loop
  return signal_loop(serial_port, phrases, n_phrases);
}

/**
 * Signal loop
 * @param serial_port The serial port file descriptor
 * @return EXIT_SUCCESS if successful, EXIT_FAILURE otherwise
 */
int signal_loop(int serial_port, char *phrases[], int n_phrases) {
  // Set up the pollfd struct
  int pret;
  struct pollfd pfds[1] = {0};
  pfds[0].fd = serial_port;
  pfds[0].events = POLLIN;

  // Read from the serial port
  int n;              // The number of bytes read
  char buf[BUF_SIZE]; // The buffer to store the data

  // Read from the serial port until the program is terminated
  while (1) {
    pret = poll(pfds, 1, POLL_TIMEOUT);
    if (pret < 0) {
      printf("Error with poll = %s\n", strerror(errno));
      return EXIT_FAILURE;
    }

    // If no data is available, sleep for a short time
    if (pret == 0) {
      usleep(SLEEP_TIME);
      continue;
    }

    // Read the data from the serial port
    n = read(serial_port, buf, BUF_SIZE);
    if (n == -1) {
      printf("Error with read = %s\n", strerror(errno));
      continue;
    }

    if (buf[0] == '0') {
      printf("[INFO] Setting up loop\n");
      if (setup_loop(serial_port) == EXIT_FAILURE) {
        printf("Error with setup_loop = %s\n", strerror(errno));
        return EXIT_FAILURE;
      }
      printf("[INFO] Loop setup complete\n");
      continue;
    }

    if (buf[0] != '\n' && buf[0] != ' ') {
      handle_button(atoi(buf), phrases, n_phrases);
    }
  }
  close(serial_port);
  return EXIT_SUCCESS;
}

/**
 * Handle the button
 * @param button The button number
 */
void handle_button(int button, char *phrases[], int n_phrases) {
  Display *display = XOpenDisplay(NULL);
  if (display == NULL) {
    printf("Error with XOpenDisplay = %s\n", strerror(errno));
    return;
  }

  if (button < 1 || button > n_phrases) {
    printf("[ERROR] Invalid button number: %d\n", button);
    XCloseDisplay(display);
    return;
  }

  // Send the phrase
  send_phrase(display, phrases[button - 1]);
  send_enter(display);

  // Close the display
  XCloseDisplay(display);
}

/**
 * Setup the loop
 * @param serial_port The serial port file descriptor
 * @return EXIT_SUCCESS if successful, EXIT_FAILURE otherwise
 */
int setup_loop(int serial_port) {
  // Send the start command
  if (write(serial_port, CMD_START, strlen(CMD_START)) == -1) {
    printf("Error with write = %s\n", strerror(errno));
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

/**
 * Open the serial port
 * @return The serial port file descriptor
 */
int open_serial_port(char *serial_port_name) {
  int serial_port = open(serial_port_name, O_RDWR | O_NOCTTY | O_NDELAY);
  if (serial_port < 0) {
    printf("Error %i from open: %s\n", errno, strerror(errno));
    return EXIT_FAILURE;
  }
  return serial_port;
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

  // Set the serial port to non-blocking
  if (fcntl(serial_port, F_SETFL, FNDELAY) == -1) {
    printf("Error with fcntl = %s\n", strerror(errno));
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}