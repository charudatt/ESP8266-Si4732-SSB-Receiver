# ESP8266 Si4732 SSB HF Receiver

A compact, open-source HF receiver project built around an **ESP8266** and **Si4732/Si4735-D60-class DSP receiver**, with local rotary control, a 1.3" SH1106 OLED, and a browser-based Wi-Fi control panel.

> **Project status:** Experimental / builder project. The firmware is intended for radio experimenters and is not a commercial receiver firmware.

### MUTE
The Web Control Interface includes a dedicated MUTE ON/OFF control. The mute state is also reflected in receiver status and the OLED READY/MUTE indication.

## Highlights

- AM, LSB and USB reception
- SSB patch loading through the PU2CLR SI4735 Arduino library
- 150 kHz–30 MHz tuning range used by the firmware
- 80 m, 40 m, 20 m, 15 m and 10 m amateur-band presets
- MW preset and an ALL-band tuning range
- Rotary encoder tuning
- Short press: mode selection
- Long press: BFO tuning mode
- BFO adjustment in 10 Hz steps, ±16 kHz
- Selectable SSB and AM bandwidths
- RSSI and SNR reporting
- 10 memory channels with EEPROM storage
- Configurable Wi-Fi Web Control: AP-first default or forced Station (STA) mode
- AP-mode **WiFi Manager** button for connecting the receiver to an existing local Wi-Fi network
- Persistent WiFiManager credentials: after successful setup, subsequent boots automatically try STA; if unavailable, the receiver falls back to AP
- Direct frequency entry, tuning buttons, step selection, AM/FM/LSB/USB mode control, volume, mute, BFO and bandwidth control
- Frozen 128×64 OLED status display with firmware version
- Web Control Interface structured to match the project documentation illustration
- Documented OLED/Web Interface layout illustration
- Settings validation when reading EEPROM
- OLED redraw designed to avoid the full-screen blink seen in earlier revisions

## Hardware

| Function | ESP8266 GPIO | NodeMCU/Wemos |
|---|---:|---|
| I2C SCL | GPIO5 | D1 |
| I2C SDA | GPIO4 | D2 |
| Si4732 RESET | GPIO2 | D4 |
| Encoder A | GPIO14 | D5 |
| Encoder B | GPIO12 | D6 |
| Encoder switch | GPIO13 | D7 |

The OLED and Si4732 share the I2C bus.

**Use 3.3 V logic for the Si4732 and OLED.** The PU2CLR library documentation also warns against applying 5 V I2C/digital signals to the Si47XX device. citeturn0search1

## Software

### Required

- Arduino IDE 1.8.x or 2.x
- ESP8266 Arduino core
- PU2CLR SI4735 Arduino library
- U8g2 library
- WiFiManager library by tzapu (required for STA-mode credential management)

The PU2CLR library supports SSB operation on Si4735-D60 and Si4732-A10 devices and provides the SSB patch-loading API used by this project. citeturn0search1

### SSB patch

The sketch currently includes:

```cpp
#include <patch_init.h>
```

Patch header names can differ between releases/forks of the PU2CLR library. The upstream library documentation also documents `patch_ssb_compressed.h` and the compressed-patch download method. citeturn0search1

If your installed library does not provide `patch_init.h`, use the patch header supplied by your installed PU2CLR library and adjust the loading call accordingly.

## Configuration

User-editable values are now separated from the hardware map:

- `pin.h` — hardware GPIO assignment
- `Config.h` — Web Wi-Fi mode, AP/STA credentials, WiFiManager settings, startup frequency, volume, splash time and default memories
- `version.h` — firmware version

This makes the project easier to publish and maintain on GitHub.

### Wi-Fi Web Control

The Web Control Interface uses an **AP-first / remembered-STA** design by default. This means the receiver is always easy to reach on first use, while a successful WiFiManager setup can make it automatically join your normal local network on future boots.

In `Config.h`:

```cpp
#define WEB_UI_WIFI_MODE_AP   0
#define WEB_UI_WIFI_MODE_STA  1
#define WEB_UI_WIFI_MODE      WEB_UI_WIFI_MODE_AP
```

#### Default: AP-first mode

With the default setting:

```cpp
#define WEB_UI_WIFI_MODE      WEB_UI_WIFI_MODE_AP
```

The first boot starts the receiver AP:

- SSID: `Si4732-Rx`
- Password: `12345678`
- Web interface: `http://192.168.4.1`

The Web Control Interface contains a **WiFi Manager** button. Pressing it starts the WiFiManager configuration portal. Connect to `Si4732-Setup`, select your local Wi-Fi network and enter its password.

After a successful configuration the receiver stores a small local flag and WiFiManager retains the network credentials. The ESP8266 then restarts. On the next boot it automatically attempts **STA mode** using the saved network.

If the saved network is unavailable, the receiver does **not** become inaccessible: it falls back to the normal `Si4732-Rx` AP and the WiFi Manager button remains available.

#### Forced STA mode

For installations where the receiver should always prefer the existing local network, change only this line:

```cpp
#define WEB_UI_WIFI_MODE      WEB_UI_WIFI_MODE_STA
```

The receiver first tries `STA_SSID`/`STA_PASSWORD` if supplied, otherwise WiFiManager-saved credentials. If no credentials are available, WiFiManager is started automatically. If configuration or connection fails, the receiver falls back to AP mode.

Optional fixed credentials are supported:

```cpp
#define STA_SSID       "YourWiFiName"
#define STA_PASSWORD   "YourWiFiPassword"
```

For most users, leave these empty and use the Web Interface **WiFi Manager** button.

#### Simple operating sequence

```text
Power ON
   │
   ├── No WiFiManager configuration yet ──> AP mode
   │                                        Si4732-Rx
   │                                             │
   │                                      WiFi Manager button
   │                                             │
   │                                      Configure local Wi-Fi
   │                                             │
   │                                      Success → Restart
   │                                             │
   │                                      Next boot → STA
   │
   └── Saved Wi-Fi available ───────────────> Try STA
                                                │
                                  Success ──────┤────> STA
                                                │
                                  Failure ──────┘────> AP
```

The frozen OLED layout is unchanged. Row 6 shows the active Web Interface IP address regardless of AP or STA mode. SSID, RSSI and SNR remain off the OLED.

The Web Interface retains the documented layout and now adds the **WiFi Manager** action below the bandwidth control.

#### Default AP mode

- SSID: `Si4732-Rx`
- Password: `12345678`
- Address: `http://192.168.4.1`

This mode is ideal for standalone operation and requires no router or Internet connection.

#### STA mode

Set:

```cpp
#define WEB_UI_WIFI_MODE      WEB_UI_WIFI_MODE_STA
```

Optional credentials may be supplied in `Config.h`:

```cpp
#define STA_SSID       "YourWiFiName"
#define STA_PASSWORD   "YourWiFiPassword"
```

If these are empty or the connection fails, the firmware starts **WiFiManager**. The operator can connect to the configuration AP, select the required Wi-Fi network and enter its credentials. WiFiManager can retain the credentials for subsequent starts. citeturn0search0turn0search1

If configuration times out or fails, the receiver falls back to its normal Soft-AP so the Web Interface remains accessible. See [`docs/WIFI_MODES.md`](docs/WIFI_MODES.md) for the complete procedure.

**Change the example passwords before permanent deployment.**

## Local controls

### Rotary encoder

- Rotate: tune frequency
- Short press: cycle LSB → USB → AM
- Long press: enable/disable BFO mode
- BFO mode: rotation changes BFO by 10 Hz

The BFO range follows the Si4735 library documentation: approximately -16.383 kHz to +16.383 kHz; this project limits it to ±16 kHz. citeturn0search0

## OLED and Web Interface

![OLED and Web Interface](docs/display_web_interface.png)

The illustration above documents the frozen OLED layout and the **v1.2.1 Web Control Interface**. The browser controls are intentionally structured to match this documentation image.

### Web interface layout

The control page contains, in order:

1. Frequency, Mode and Step
2. Four frequency tuning buttons
3. AM / FM / LSB / USB mode buttons
4. Volume slider and numeric value
5. BFO ON/OFF control, centred −16,000 to +16,000 Hz slider and numeric value
6. Bandwidth selector
7. Receiver status including RSSI, SNR, Si4732 detection, Wi-Fi mode/IP and uptime

The RESET control has been removed from the Web Interface. The Web Control Interface includes M1–M10 STORE and RECALL controls; the memory dropdown identifies each slot by stored frequency and mode (or Empty).

For the exact OLED row allocation, see [`docs/OLED_AND_WEB_INTERFACE.md`](docs/OLED_AND_WEB_INTERFACE.md). For Wi-Fi operating modes, see [`docs/WIFI_MODES.md`](docs/WIFI_MODES.md).

## Web API

### `GET /`

Returns the control page.

### `GET /status`

Returns JSON containing:

- frequency
- mode
- volume
- step
- RSSI
- SNR
- band
- mute state
- BFO
- BFO state
- SSB bandwidth
- AM bandwidth
- firmware version

### `GET /cmd?a=...&v=...`

Supported actions include:

| Action | Example |
|---|---|
| tune | `/cmd?a=tune&v=1` |
| freq | `/cmd?a=freq&v=14200` |
| mode | `/cmd?a=mode&v=USB` |
| band | `/cmd?a=band&v=1` |
| step | `/cmd?a=step&v=5` |
| vol | `/cmd?a=vol&v=2` |
| volset | `/cmd?a=volset&v=40` |
| mute | `/cmd?a=mute&v=toggle` |
| bfo | `/cmd?a=bfo&v=10` |
| bfo | `/cmd?a=bfo&v=toggle` |
| bwssb | `/cmd?a=bwssb&v=2` |
| bwam | `/cmd?a=bwam&v=3` |
| mem_store | `/cmd?a=mem_store&v=0` |
| mem_recall | `/cmd?a=mem_recall&v=0` |

`/mem` returns the ten memory entries as JSON.

## EEPROM

The firmware stores:

- volume
- band
- mode
- frequency
- BFO
- tuning step
- SSB bandwidth
- AM bandwidth
- memory channels

The settings loader validates important values before using them. A corrupt or out-of-range frequency is replaced by the selected band's default frequency.

## Repository structure

```text
ESP8266_Si4732_SSB_Rx/
├── ESP8266_Si4732_SSB_Rx.ino
├── Config.h
├── pin.h
├── version.h
├── Rotary.h
├── Rotary.cpp
├── README.md
├── CHANGELOG.md
├── LICENSE
├── .gitignore
└── docs/
    ├── HARDWARE.md
    ├── WEB_API.md
    ├── PROJECT_STRUCTURE.md
    ├── FEATURE_ROADMAP.md
    ├── OLED_AND_WEB_INTERFACE.md
    ├── WIFI_MODES.md
    └── display_web_interface.png
```

## Known limitations

1. SSB patch support depends on the exact Si4732/Si4735 device and PU2CLR library/patch version.
2. The web server uses the ESP8266's simple `ESP8266WebServer`; in STA mode use it only on a trusted local network and do not expose it directly to the Internet.
3. The current project does not yet include AGC control, bandstacking, CAT control, or an audio DSP chain. The OLED layout shown in `docs/OLED_AND_WEB_INTERFACE.md` is frozen; the Web Control Interface layout is frozen to the documented v1.2.1 structure.
4. No claim is made that every Si4732 module has identical RF performance. Front-end filtering, antenna matching, grounding and power supply quality remain important.

## Suggested development path

See [`docs/FEATURE_ROADMAP.md`](docs/FEATURE_ROADMAP.md). The recommended next stages are:

1. Band-specific frequency/step memory
2. Better signal meter and SNR/RSSI history
3. AGC and soft-mute controls
4. Bandstacking
5. Scan / seek
6. CAT-style control API
7. Wi-Fi Station mode refinements and reconnect handling
8. Improved mobile web UI
9. Receiver calibration/alignment tools
10. Integration with the larger ESP32-S3 SDR project

## Credits

- **PU2CLR / Ricardo Lima Caratti** — SI4735 Arduino library and SSB support
- **Ben Buxton** — Rotary encoder state-machine implementation included in this repository
- **Oli Kraus** — U8g2/U8x8 display library

The PU2CLR SI4735 library is distributed under the MIT license. citeturn0search1

The included Ben Buxton Rotary implementation carries its original GPL licensing notice; therefore this repository uses GPL-3.0-or-later for the combined project.

## License

GPL-3.0-or-later. See [`LICENSE`](LICENSE).

## Disclaimer

This is an amateur-radio/homebrew project. Verify your local radio regulations, transmitter controls and RF safety requirements before connecting this receiver/controller to other equipment.

### Memory controls
The Web Interface includes M1–M10 STORE and RECALL controls. The selected memory slot shows its stored frequency and mode.
