# Project Structure

## Main sketch

`ESP8266-Si4732-SSB-Receiver.ino`

Contains:

- receiver state
- setup/loop
- Si4732 configuration
- EEPROM handling
- OLED rendering
- web server and web UI

## Configuration

`pin.h`

Hardware-only GPIO map.

`Config.h`

User-facing settings:

- Web Wi-Fi mode, Soft-AP credentials, STA credentials and WiFiManager settings
- startup frequency
- default volume
- splash duration
- memory defaults

`version.h`

Single source for firmware version.

## Rotary

`Rotary.h` / `Rotary.cpp`

Ben Buxton's quadrature rotary encoder state machine.

## Why keep the sketch together?

For this project size, keeping the application logic in one `.ino` makes the Arduino IDE workflow simple for builders. The configuration and hardware definitions are already isolated, so the next major refactor should only be done when a new subsystem (CAT, scan engine, DSP, etc.) justifies separate modules.
