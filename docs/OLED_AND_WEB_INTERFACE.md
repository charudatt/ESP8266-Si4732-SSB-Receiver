# OLED and Web Interface

## Frozen OLED display layout — firmware v1.2.1

The OLED layout is frozen. The combined illustration below is the project documentation reference for both the display and the browser controls.

![OLED and Web Interface](display_web_interface.png)

The 128×64 OLED uses the following layout:

| OLED row | Content |
|---|---|
| 0–1 | Frequency in large font + Mode in smaller font, with one character space between them |
| 2 | `Step: XX` and `V:XX` |
| 3 | `BFO: ON/OFF` and selected `BW` |
| 4 | Blank |
| 5 | Blank / explicitly cleared |
| 6 | Active Web Interface IP address |
| 7 | Firmware version and status |

The OLED intentionally does **not** show SSID, RSSI, SNR or Band.

Example:

```text
14200 LSB
Step:  1  V:47
BFO: OFF BW:3.0


192.168.4.1
v1.2.1 READY
```

The OLED rows that are not used are explicitly cleared so that text from a previous screen cannot remain visible.

## Web interface

The primary browser control page is structured to match the documentation illustration:

1. Frequency, Mode and Step
2. Four tuning buttons
3. AM / FM / LSB / USB mode buttons
4. Volume slider and value
5. BFO ON/OFF, centred −16,000 to +16,000 Hz slider and numeric value
6. Bandwidth selector
7. WiFi Manager button for local-network configuration
8. Receiver status area with RSSI, SNR, Si4732 status, Wi-Fi mode/IP and uptime

The RESET control is intentionally absent.

### BFO control

The BFO is controlled by a slider-style control with:

- Range: −16,000 Hz to +16,000 Hz
- Resolution: 10 Hz
- Numeric value display
- BFO enable/disable control

The BFO control is intended primarily for SSB operation.

### Bandwidth

The web interface provides selectable SSB and AM bandwidth settings. The selected bandwidth is also shown beside the BFO state on the OLED.

## Wi-Fi modes

The Web Control Interface supports two operating modes selected in `Config.h`:

```cpp
#define WEB_UI_WIFI_MODE_AP   0
#define WEB_UI_WIFI_MODE_STA  1
#define WEB_UI_WIFI_MODE      WEB_UI_WIFI_MODE_AP
```

### AP-first mode — default

The ESP8266 creates the local receiver network:

- SSID: `Si4732-Rx`
- Password: `12345678`
- Web address: `http://192.168.4.1`

The Web Interface includes a **WiFi Manager** button. Use it to connect the receiver to an existing local Wi-Fi network. After successful configuration, the receiver restarts and automatically attempts STA mode on later boots.

If the saved network is unavailable, the receiver falls back to the normal `Si4732-Rx` AP.

### STA mode

Set:

```cpp
#define WEB_UI_WIFI_MODE WEB_UI_WIFI_MODE_STA
```

The firmware prefers STA at startup using `Config.h` credentials or WiFiManager-saved credentials. If no usable connection is available, WiFiManager can start automatically; if configuration times out, the receiver falls back to AP.

See [`WIFI_MODES.md`](WIFI_MODES.md) for the complete configuration procedure.

See [`WIFI_MODES.md`](WIFI_MODES.md) for the complete configuration procedure.

## Documentation image

The combined illustration above is intended for GitHub README pages, project documentation and builder manuals. It represents the documented receiver UI structure; the values shown are example operating values.
