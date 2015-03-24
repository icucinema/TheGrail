// Curtain controller for connection to Goelst G-Rail systems

// Expects curtain to respond on "B" channel on 1 open/close

// Written by: Luke Granger-Brown and George Goldberg

#include <stdint.h>

#define PIN PIN_B0
#define PULSE_WIDTH_US 889

#define CURTAIN_ADDRESS 0x1D
#define CMD_OPEN 0x01
#define CMD_CLOSE 0x21
#define CMD_PRESET_1 0x18
#define CMD_PRESET_2 0x19
#define CMD_PRESET_3 0x1A
#define CMD_PRESET_4 0x1B
#define CMD_PRESET_5 0x1C

#define PIN_BTN_CLOSE PIN_A0
#define PIN_BTN_SCOPE PIN_A1
#define PIN_BTN_FLAT PIN_A2
#define PIN_BTN_169 PIN_A3
#define BTN_COUNT 4

#define DELAY_BETWEEN_RPT_MS 100 // ms
#define DELAY_BETWEEN_CMD_MS 500-DELAY_BETWEEN_RPT_MS // ms

#define SEND_BIT 0x2000

int written;
unsigned toggleBit;
bool btnState[BTN_COUNT];

unsigned btnPins[] = {
  PIN_BTN_CLOSE, PIN_BTN_SCOPE, PIN_BTN_FLAT, PIN_BTN_169,
};
unsigned btnCmd[][3] = {
  {CMD_CLOSE, 0, 0},
  {CMD_OPEN, 0, 0},
  {CMD_PRESET_1, CMD_OPEN, 0},
  {CMD_PRESET_2, CMD_OPEN, 0},
};

void setup() {
  pinMode(PIN, OUTPUT);
  pinMode(PIN_BTN_CLOSE, INPUT);
  pinMode(PIN_BTN_SCOPE, INPUT);
  pinMode(PIN_BTN_FLAT, INPUT);
  pinMode(PIN_BTN_169, INPUT);
  
  digitalWrite(PIN, HIGH);
  
  written = 0;
  toggleBit = 0;
  for (int i = 0; i < BTN_COUNT; i++) {
    btnState[i] = false;
  }
}

inline uint16_t composeMessage(unsigned toggleBit, unsigned addr, unsigned cmd) {
  return (0x3 << 12 | toggleBit << 11 | addr << 6 | cmd);
}

inline void sendMessage(uint16_t message) {
  for (int i = 0; i < 14; i++) {
    if ((message & SEND_BIT) == SEND_BIT) {
      digitalWrite(PIN, HIGH);
      delayMicroseconds(PULSE_WIDTH_US);
      digitalWrite(PIN, LOW);
      delayMicroseconds(PULSE_WIDTH_US);
    } else {
      digitalWrite(PIN, LOW);
      delayMicroseconds(PULSE_WIDTH_US);
      digitalWrite(PIN, HIGH);
      delayMicroseconds(PULSE_WIDTH_US);
    }
    message <<= 1;
  }
  // return high
  digitalWrite(PIN, HIGH);
}

void composeAndSend(unsigned* toggleBit, unsigned addr, unsigned cmd) {
  sendMessage(composeMessage(*toggleBit, addr, cmd));
  delay(DELAY_BETWEEN_CMD_MS);
  *toggleBit = !*toggleBit;
}

// the loop function runs over and over again forever
void loop() {
  for (int i = 0; i < BTN_COUNT; i++) {
    int pin = btnPins[i];
    bool pressed = digitalRead(pin) == HIGH;
    bool lastState = btnState[i];
    if (lastState != pressed && pressed) {
      for (int j = 0; j < 3; j++) {
        if (btnCmd[i][j] != 0) {
          composeAndSend(&toggleBit, CURTAIN_ADDRESS, btnCmd[i][j]);
        }
      }
    }
    btnState[i] = pressed;
  }
}
