/*
 * Rotary encoder library for Arduino.
 * Copyright 2011 Ben Buxton. Licenced under the GNU GPL Version 3.
 * Contact: bb@cactii.net
 *
 * A typical mechanical rotary encoder emits a two bit gray code
 * on 2 output channels. Each time the encoder is physically rotated
 * by one detent (click), the A and B signals go through a specific
 * sequence of transitions from 00 to 01 to 11 to 10 and back to 00.
 *
 * To get this to work reliably, the code needs to be run frequently
 * enough that it can sample the signal transitions. On a high-speed
 * MCU this can be done in the main loop, but on slower processors
 * an interrupt service routine is required.
 */

#ifndef Rotary_h
#define Rotary_h

#include "Arduino.h"

// Enable this to emit codes twice per step.
// #define HALF_STEP

// Enable weak pullups
#define ENABLE_PULLUPS

// Values returned by process()
#define DIR_NONE 0x0
#define DIR_CW   0x10
#define DIR_CCW  0x20

class Rotary
{
  public:
    Rotary(char, char);
    unsigned char process();
    void begin(bool pullup = true);
  private:
    unsigned char state;
    unsigned char pin1;
    unsigned char pin2;
};

#endif
