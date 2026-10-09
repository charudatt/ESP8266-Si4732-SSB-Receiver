# OLED and Web Control Interface

## Frozen OLED layout — v1.2.4

The OLED display is intentionally frozen for the current project baseline.

| OLED row | Content |
|---|---|
| 0–1 | Large frequency + reduced-size mode, with one character space between them |
| 2 | Step + Volume; `V:XX` is shifted two character spaces to the right |
| 3 | BFO + Bandwidth (`BW`) |
| 4 | Blank / cleared |
| 5 | Blank / cleared |
| 6 | Active Web Interface IP address |
| 7 | Firmware version + status |

The OLED does **not** show SSID, RSSI, SNR or Band.

## Web Control Interface — v1.2.4

The Web Interface is designed for both phones/tablets and larger screens. Its main controls are:

1. Frequency / mode / step information
2. Tuning controls
3. AM / FM / LSB / USB mode selection
4. Volume control
5. **MUTE** control
6. BFO ON/OFF and centred BFO slider
7. Bandwidth selection
8. Memory selector with the stored frequency and mode in every dropdown entry
9. **STORE** and **RECALL** memory controls
10. WiFi Manager
11. Receiver status including RSSI, SNR, Wi-Fi mode, IP and uptime

The RESET control was deliberately removed.

## Memory selector

Each dropdown entry identifies the actual stored state, for example `M1 — 7.074 MHz LSB`. Empty channels are shown as `Empty`. This avoids having to guess which frequency is stored in a memory channel.

After STORE, the selector is refreshed so the newly stored frequency/mode is immediately visible.

## Wi-Fi controls

The Web Interface can operate in AP or STA mode. The default project configuration is AP-first. WiFiManager can be invoked from the Web Interface to configure a local network. Once valid credentials have been saved, the receiver can operate in STA mode; if the network cannot be reached it falls back to AP mode.

![Approved OLED and Web Interface](display_web_interface.png)
