/* Rotary encoder handler for arduino. v1.1
 *
 * Copyright 2011 Ben Buxton. Licenced under the GNU GPL Version 3.
 * Contact: bb@cactii.net
 */

#include "Arduino.h"
#include "Rotary.h"

/*
 * The below state table has, for each state (row), the new state
 * to go to based on the next encoder output. From left to right,
 * it is 00, 01, 10, 11 (the original state is the row number).
 *
 * The state machine is from Ben Buxton's original library.
 */

#ifdef HALF_STEP
// Use the half-step state table (emits a code at 00 and 11)
const unsigned char ttable[6][4] = {
  {0x3, 0x2, 0x1, 0x0}, {0x23, 0x0, 0x1, 0x0},
  {0x13, 0x2, 0x0, 0x0}, {0x3, 0x5, 0x4, 0x0},
  {0x3, 0x2, 0x4, 0x10}, {0x3, 0x5, 0x4, 0x20}
};
#else
// Use the full-step state table (emits a code at 00 only)
const unsigned char ttable[7][4] = {
  {0x0, 0x2, 0x4, 0x0}, {0x3, 0x0, 0x1, 0x10},
  {0x3, 0x2, 0x0, 0x0}, {0x3, 0x2, 0x1, 0x0},
  {0x6, 0x0, 0x4, 0x0}, {0x6, 0x5, 0x0, 0x20},
  {0x6, 0x5, 0x4, 0x0}
};
#endif

Rotary::Rotary(char _pin1, char _pin2) {
  pin1 = _pin1;
  pin2 = _pin2;
  state = 0;
}

void Rotary::begin(bool pullup) {
  if (pullup) {
    pinMode(pin1, INPUT_PULLUP);
    pinMode(pin2, INPUT_PULLUP);
  } else {
    pinMode(pin1, INPUT);
    pinMode(pin2, INPUT);
  }
}

unsigned char Rotary::process() {
  unsigned char pinstate = (digitalRead(pin2) << 1) | digitalRead(pin1);
  state = ttable[state & 0xf][pinstate];
  return (state & 0x30);
}
