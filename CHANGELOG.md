# Changelog

## v1.3.0
- Fixed Web BFO slider updates so the OLED immediately reflects the selected BFO value.
- Web `bfo` and `bfoSet` commands now refresh the OLED after applying the value.
- Preserved the working memory RECALL, direct frequency SET, BAND - / BAND +, STORE, MUTE, bandwidth, volume and WiFi Manager controls.

## v1.2.9
- Restored the missing **SET** button beside the Web Control Interface frequency entry.
- Frequency can be entered manually and applied explicitly with **SET** (or Enter).
- The 1-second status refresh no longer overwrites the frequency field while the user is editing it.
- Preserved working memory RECALL, BAND - / BAND +, STORE, MUTE, BFO, bandwidth, volume and WiFi Manager controls.


## v1.2.8
- Fixed Web Memory selection so changing the dropdown does **not** tune the receiver.
- Fixed/validated memory EEPROM handling with a format marker and corruption checks.
- Invalid legacy memory data (including impossible values such as 4 kHz) is repaired from `MEM_TABLE`.
- Added explicit UTF-8 declaration for the Web UI.
- Preserved working BAND UP / BAND DOWN controls and all v1.2.6 features.

## v1.2.8

- Restored dedicated **BAND - / BAND +** controls to the Web Control Interface.
- The controls use the existing band-selection API and do not alter the frozen OLED layout.
- Preserved the v1.2.5 memory dropdown, STORE/RECALL, MUTE, BFO, bandwidth, volume and WiFi Manager functionality.

## v1.2.5

- Fixed the Web Interface memory selector so every dropdown entry shows the stored frequency and mode, e.g. `M1 — 7.074 MHz LSB`, or `Empty` when unused.
- STORE and RECALL refresh the dropdown immediately.

## v1.2.4
- Restored the dedicated MUTE control to the Web Control Interface.
- MUTE state is shown as MUTE ON/OFF and remains synchronized with receiver status.

## v1.2.3
- Memory dropdown now shows each stored frequency and mode (or Empty) for M1–M10.

## v1.2.2
- Restored Web Interface STORE and RECALL memory controls for M1–M10.
- Added selected-memory status display.

# Changelog

## v1.2.1

- Added AP-first / remembered-STA Wi-Fi behavior.
- Added **WiFi Manager** button to the Web Control Interface.
- WiFiManager can now be launched directly from the normal `Si4732-Rx` AP.
- After successful WiFiManager configuration, the receiver restarts and automatically attempts STA on subsequent boots.
- If the saved local network is unavailable, the receiver falls back to the normal AP.
- Added persistent STA-configured marker in EEPROM.
- Kept the frozen OLED layout unchanged.
- Updated Wi-Fi documentation and README.

## 1.2.0
- Added configurable Web Control Wi-Fi mode in `Config.h`.
- Default remains local Soft-AP mode.
- Added optional Station (STA) mode for use on an existing LAN/Wi-Fi network.
- Added WiFiManager captive-portal support when STA credentials are unavailable or a configured network cannot be reached.
- Added automatic fallback to the local Soft-AP if STA configuration times out or fails.
- Added active Wi-Fi mode and IP address to `/status`.
- Added Si4732 detection status and uptime to `/status`.
- Rebuilt the primary Web Control Interface to match the project documentation illustration.
- Added frequency, mode, step, tuning, volume, BFO, bandwidth and receiver-status layout in the documented order.
- Removed the primary memory-control section from the browser page; memory API support remains available.
- Kept the frozen OLED layout unchanged.

## 1.1.8
- Reduced OLED MODE font to the normal 8x8 font while keeping it on the frequency line.
- Added exactly one character space between the large frequency and MODE.
- Shifted the OLED volume field two character spaces to the right.
- Added SSB bandwidth (BW) beside the BFO value on OLED row 3.

## 1.1.7
- Corrected OLED status row allocation.
- Row 3 contains BFO/BW.
- Rows 4 and 5 are blank and explicitly cleared.
- Row 6 contains the IP address.
- Row 7 contains firmware/status.

## 1.1.6
- Fixed OLED status-row cleanup to prevent stale characters from a previous screen.

## 1.1.3–1.1.5
- Simplified and corrected the OLED frequency/mode/step/volume layout.
- Removed BAND, SSID, RSSI and SNR from the OLED.

## 1.1.1
- Removed the RESET button from the Web Interface.
- Replaced the Web Interface BFO increment/decrement controls with a centred -16 kHz to +16 kHz slider in 10 Hz steps.
- Added an absolute `bfoSet` Web API action used by the slider.

## 1.1.0 — GitHub preparation
- Added `Config.h` for user-editable settings.
- Added `version.h` for firmware versioning.
- Added repository documentation and feature roadmap.
- Isolated hardware pin definitions in `pin.h`.
- Added EEPROM range validation.
- Reduced OLED flicker by avoiding full-screen redraws during normal tuning.
