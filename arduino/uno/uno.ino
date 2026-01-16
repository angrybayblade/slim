#include <SPI.h>
#include <Wire.h>

// 16 bit multiplexer
#include <CD74HC4067.h>

// Adafruit fingerprint
#include <Adafruit_Fingerprint.h>

// Loop config
#define L_STATE_INIT 0
#define L_STATE_CONFIG 1
#define L_STATE_READ 2

// Mux config
#define ANALOG_READ_S 512

#define SIG_PIN A6
#define SIG_0 5
#define SIG_1 4
#define SIG_2 3
#define SIG_3 2

CD74HC4067 btn_mux(SIG_0, SIG_1, SIG_2, SIG_3);  // (S0, S1, S2, S3)

// Pin config
#define NUMBER_OF_PINS 3
uint8_t pin_states[NUMBER_OF_PINS];

// Fingerprint sensor config
#define FGP_TXS 11  // yellow wire
#define FGP_RXS 12  // green wire

// Setup fingerprint
#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
// For UNO and others without hardware serial, we must use software serial...
// pin #11 is IN from sensor (YELLOW wire)
// pin #12 is OUT from arduino  (GREEN wire)
// Set up the serial port to use softwareserial..
SoftwareSerial fgpSerial(FGP_TXS, FGP_RXS);

#else
// On Leonardo/M0/etc, others with hardware serial, use hardware serial!
// #0 is green wire, #1 is white

#define fgpSerial Serial1
#endif

// Fingerprint reader
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fgpSerial);

// Serial coms
// ┌─────────────────────────┐
// |SER_F_XXX|SIZE_HEADER|MSG|
// └─────────────────────────┘
#define SER_F_LOG 0  // log message
#define SER_F_STP 1  // setup requirement
#define SER_F_BTN 2  // button press

#define WRITE_LOG_SX "%d%d %s"
#define WRITE_LOG_SXX "%d%d%s"
#define WRITE_BTN_SX "%d%d %d"
#define WRITE_BTN_SXX "%d%d%d"

void write_log(char *text) {
  size_t size = strlen(text);
  size_t package_size = 1 + 2 + size + 1;
  char msg[package_size];
  if (size < 10) {
    sprintf(msg, WRITE_LOG_SX, SER_F_LOG, size, text);
  } else {
    sprintf(msg, WRITE_LOG_SXX, SER_F_LOG, size, text);
  }
  Serial.print(msg);
  Serial.flush();
}

void write_btn(int flag, int btn) {
  Serial.print(SER_F_BTN);
  size_t package_size;
  if (btn < 10) {
    package_size = 1 + 2 + 1 + 1;
  } else {
    package_size = 1 + 2 + 2 + 1;
  }

  char msg[package_size];
  if (btn < 10) {
    sprintf(msg, WRITE_BTN_SXX, SER_F_LOG, 1, btn);
  } else {
    sprintf(msg, WRITE_BTN_SXX, SER_F_LOG, 2, btn);
  }
  Serial.print(msg);
  Serial.flush();
}

void _print_fingerprint_info(void) {
  // Print fingerprint sensor info
  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial.print(F("Status: 0x"));
  Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x"));
  Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: "));
  Serial.println(finger.capacity);
  Serial.print(F("Security level: "));
  Serial.println(finger.security_level);
  Serial.print(F("Device address: "));
  Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: "));
  Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: "));
  Serial.println(finger.baud_rate);
}

void setup_fingerprint(void) {
  // dsp_text(1, WHITE, 36, 28, "FGP setup", DSP_CLEAR);
  write_log("checking fingerprint sensor");

  // set the data rate for the sensor serial port
  finger.begin(57600);
  if (finger.verifyPassword()) {
    write_log("found fingerprint sensor");
    // dsp_text(1, WHITE, 36, 28, "FGP found", DSP_CLEAR);
    return;
  }

  // Wait until fingerprint sensor is detected
  // dsp_text(1, WHITE, 24, 28, "FGP not found!", DSP_CLEAR);
  write_log("fingerprint sensor not found");
  while (1) { delay(1); }
}

void check_fingerprints_exist(void) {
  if (fgp_list()) {
    write_log("found valid fingerprints");
    return;
  }

  while (!fgp_enroll()) { delay(100); }
}

uint8_t read_int(void) {
  uint8_t num = 0;
  while (num == 0) {
    while (!Serial.available());
    num = Serial.parseInt();
  }
  return num;
}

void setup() {
  Serial.begin(9600);
  Serial.flush();

  // // setup fingerprint sensors
  // setup_fingerprint();
  // delay(100);

  // // validate fingerprints
  // check_fingerprints_exist();
  // delay(100);
  write_log("finished setup");
}

void loop() {
  // equivelant of defer
  delay(10);

  // NOTE: this is a blocking operation, this makes an assumption that
  // one key will be pressed at the time. In future, can change this to
  // press and release signals.
  int btn = btn_read();
  if (btn == -1) {
    return;
  }

  // // validate fingerprint
  // if (!fgp_detect()) {
  //   return;
  // }

  // log button
  write_btn(SER_F_BTN, btn);
}


void btn_debug(int i) {
  Serial.print(i);
  Serial.print("->");
  Serial.print(analogRead(SIG_PIN));
  Serial.print(analogRead(SIG_PIN) / ANALOG_READ_S);
  Serial.print(" ");
}

int btn_read(void) {
  int btn_read = -1;
  for (int i = 0; i < NUMBER_OF_PINS; i++) {
    btn_mux.channel(i);
    // btn_debug(i);
    while ((analogRead(SIG_PIN) / ANALOG_READ_S) > 0.0) {
      pin_states[i] = 1;
    }

    if (pin_states[i]) {
      btn_read = i;
    }

    pin_states[i] = 0;
    if (btn_read > -1) {
      return btn_read;
    }
  }
  Serial.println();
  return btn_read;
}

int fgp_list(void) {
  finger.getTemplateCount();
  if (finger.templateCount == 0) {
    return 0;
  }
  return 1;
}

uint8_t fgp_enroll() {
  // Just one user allowed at a time
  uint8_t id = 1;
  int p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        write_log("image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print("no finger detected");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        write_log("communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        write_log("imaging error");
        break;
      default:
        write_log("unknown error");
        break;
    }
    delay(100);
  }

  // OK success!
  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      write_log("image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      write_log("image too messy");
      return 0;
    case FINGERPRINT_PACKETRECIEVEERR:
      write_log("communication error");
      return 0;
    case FINGERPRINT_FEATUREFAIL:
      write_log("could not find fingerprint features");
      return 0;
    case FINGERPRINT_INVALIDIMAGE:
      write_log("could not find fingerprint features");
      return 0;
    default:
      write_log("unknown error");
      return 0;
  }

  write_log("remove finger");
  delay(1000);

  p = 0;
  while (p != FINGERPRINT_NOFINGER) { p = finger.getImage(); }
  Serial.print("ID ");
  Serial.println(id);

  p = -1;
  write_log("place same finger again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        write_log("image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print("invalid fingerprint scan");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        write_log("communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        write_log("imaging error");
        break;
      default:
        write_log("unknown error");
        break;
    }
  }

  // OK success!
  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      write_log("image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      write_log("image too messy");
      return 0;
    case FINGERPRINT_PACKETRECIEVEERR:
      write_log("communication error");
      return 0;
    case FINGERPRINT_FEATUREFAIL:
      write_log("could not find fingerprint features");
      return 0;
    case FINGERPRINT_INVALIDIMAGE:
      write_log("could not find fingerprint features");
      return 0;
    default:
      write_log("unknown error");
      return 0;
  }

  // OK converted!
  write_log("creating model");

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    write_log("prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    write_log("communication error");
    return 0;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    write_log("fingerprints did not match");
    return 0;
  } else {
    write_log("unknown error");
    return 0;
  }


  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    write_log("stored fingerprint");
    return 1;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    write_log("communication error");
    return 0;
  } else if (p == FINGERPRINT_BADLOCATION) {
    write_log("could not store in that location");
    return 0;
  } else if (p == FINGERPRINT_FLASHERR) {
    write_log("error writing to flash");
    return 0;
  } else {
    write_log("unknown error");
    return 0;
  }

  return true;
}

uint8_t fgp_delete() {
  uint8_t p = -1;
  p = finger.deleteModel(1);
  if (p == FINGERPRINT_OK) {
    write_log("deleted successfully");
    return 1;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    write_log("communication error");
  } else if (p == FINGERPRINT_BADLOCATION) {
    write_log("could not delete in that location");
  } else if (p == FINGERPRINT_FLASHERR) {
    write_log("error writing to flash");
  } else {
    write_log("unknown error: 0x");
  }
  return 0;
}

uint8_t fgp_detect() {
  write_log("validating fingerprint");
  uint8_t p;
  while (1) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        write_log("image taken");
        break;
      case FINGERPRINT_NOFINGER:
        delay(10);
        continue;
      case FINGERPRINT_PACKETRECIEVEERR:
        write_log("communication error");
        return 0;
      case FINGERPRINT_IMAGEFAIL:
        write_log("imaging error");
        return 0;
      default:
        write_log("unknown error");
        return 0;
    }
    break;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      write_log("image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      write_log("image too messy");
      return 0;
    case FINGERPRINT_PACKETRECIEVEERR:
      write_log("communication error");
      return 0;
    case FINGERPRINT_FEATUREFAIL:
      write_log("could not find fingerprint features");
      return 0;
    case FINGERPRINT_INVALIDIMAGE:
      write_log("could not find fingerprint features");
      return 0;
    default:
      write_log("unknown error");
      return 0;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    write_log("found a print match");
    delay(100);
    return 1;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    write_log("communication error");
    return 0;
  } else if (p == FINGERPRINT_NOTFOUND) {
    write_log("did not find a match");
    return 0;
  } else {
    write_log("unknown error");
    return 0;
  }
}
