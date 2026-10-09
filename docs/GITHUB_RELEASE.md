# GitHub Release Information

## Project title

`ESP8266-Si4732-SSB-Receiver`

## Release baseline

Firmware: **v1.2.8**

This package is the clean documented baseline for the first public GitHub repository release.

## Suggested repository description

ESP8266-based Si4732 HF receiver with AM/FM/LSB/USB, OLED status display, rotary tuning, EEPROM memories, BFO and bandwidth controls, and a responsive Wi-Fi Web Control Interface with AP/STA and WiFiManager support.

## Suggested initial commit message

`Initial release: ESP8266-Si4732-SSB-Receiver v1.2.8`

## Suggested GitHub release title

`v1.2.8 — Web band controls restored`

## Repository notes

- The OLED layout is intentionally frozen at the v1.2.4 design.
- The Web Control Interface includes BAND - / BAND +, MUTE, BFO, bandwidth, memory STORE/RECALL, and WiFi Manager controls.
- The default Wi-Fi behavior is AP-first, with remembered STA operation after successful WiFiManager configuration and AP fallback if the saved network is unavailable.
- RESET is intentionally not exposed in the Web Control Interface.
