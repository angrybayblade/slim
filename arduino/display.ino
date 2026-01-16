// Adafruit OLED
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Display flags
#define DSP_NOCLEAR 0
#define DSP_CLEAR 1

// OLED config
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

// Declaration for SSD1306 display connected using I2C
#define OLED_RESET -1  // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C

// Declaration for SSD1306 display connected using software SPI:
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void _calibrate_display(void) {
  for (int i = 0; i < 5; i++) {
    char s[32];
    sprintf(s, "Calibrating -> %d", i);
    // dsp_text(1, WHITE, 12, 28, s, DSP_CLEAR);
    delay(100);
  }
}

void setup_display() {
  while (1) {
    if (display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
      break;
      // for (;;)
      //   ;  // Don't proceed, loop forever
    }
    write_log("Initializing failed...");
    delay(1000);
  }
  // Display message
  dsp_text(1, WHITE, 32, 28, "Initializing...", DSP_CLEAR);
  
  // Calibrate display
  _calibrate_display();
}

void dsp_text(int text_size, int color, int x, int y, char *text, int clear)
{
  if (clear) {
    dsp_clear();
  }
  Serial.println(text);
  display.setTextSize(text_size);
  display.setTextColor(WHITE);
  display.setCursor(x, y);
  display.print(text);
  display.display();
}

void dsp_clear(void) {
  display.clearDisplay();
}