# Feature Roadmap

## Implemented in v1.2.1

- AP-first / remembered-STA Wi-Fi behavior.
- Web Interface WiFi Manager button.
- Persistent successful-STA state with automatic STA attempt on later boots.
- Automatic fallback to the normal receiver AP when the saved network is unavailable.


The project should grow in small, independently testable stages.

## Priority 1 — Receiver usability

### 1. Per-band state
Store, for every band:

- last frequency
- step
- mode
- BFO
- bandwidth

This is more useful than one global frequency because switching between 40 m and 20 m should return to the last listening point on each band.

### 2. Better signal meter
Add:

- RSSI bar graph
- SNR bar graph
- peak/hold indication
- optional averaging

Keep numeric RSSI/SNR available through the API.

### 3. AGC control
Expose the receiver's AGC/AVC behavior through the web UI and, later, a local encoder menu.

### 4. Scan / seek
Add controlled scanning within a selected band with:

- start
- stop
- step
- RSSI/SNR threshold
- optional memory capture

## Priority 2 — Operator interface

### 5. Bandstacking
Provide multiple memories per amateur band.

### 6. CW receive profile
Use the 500 Hz / 1 kHz SSB bandwidth modes with dedicated CW BFO presets.

### 7. Mobile UI
Replace the large single-page layout with a compact responsive dashboard:

- frequency as the main element
- signal meter
- mode
- BFO
- bandwidth
- memories

### 8. Browser keyboard shortcuts
Examples:

- Up/Down: tune
- Page Up/Page Down: large tuning step
- L/U/A: mode
- M: mute
- B: BFO
- number keys: memory recall

## Priority 3 — Integration

### 9. CAT-style API
Add machine-readable commands suitable for logging/control software.

A simple JSON API is preferable to adding a large protocol stack initially.

### 10. Wi-Fi Station mode
Configurable AP/STA operation with WiFiManager credential entry and Soft-AP fallback is now implemented in v1.2.0.

### 11. Remote monitoring
Provide read-only status pages suitable for another device.

## Priority 4 — RF/DSP experimentation

### 12. Calibration tools
Provide controlled tests for:

- frequency readback
- BFO offset
- RSSI/SNR
- tuning accuracy

### 13. Audio processing
If the hardware audio path permits it, investigate:

- noise reduction
- audio equalization
- CW filtering
- signal recording

### 14. Front-end control
Future hardware revisions could add:

- band-pass filter switching
- RF attenuator
- preselector
- antenna selection

## Integration with the ESP32-S3 project

The most valuable long-term direction is to keep the Si4732 receiver as an **optional RF source** and expose a clean control/audio interface to the larger ESP32-S3 SDR project.

Do not merge the two projects prematurely. Validate the Si4732 receiver independently first.
