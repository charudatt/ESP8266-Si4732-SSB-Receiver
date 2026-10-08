#ifndef PIN_H
#define PIN_H

/*
 * Hardware pin map for ESP8266 NodeMCU / Wemos D1 mini.
 *
 * GPIO numbers are used in the sketch so the same map works with both
 * NodeMCU and Wemos-style ESP8266 boards.
 */

// Shared I2C bus: OLED + Si4732
#define PIN_SCL             5   // GPIO5  / D1
#define PIN_SDA             4   // GPIO4  / D2

// Si4732 RESET (active low)
#define PIN_SI4732_RST      2   // GPIO2  / D4

// Rotary encoder
#define PIN_ENC_A          14   // GPIO14 / D5
#define PIN_ENC_B          12   // GPIO12 / D6
#define PIN_ENC_SW         13   // GPIO13 / D7

// User-editable settings live in Config.h.
#include "Config.h"

#endif
