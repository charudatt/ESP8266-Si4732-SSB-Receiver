#ifndef CONFIG_H
#define CONFIG_H

/*
 * User configuration for the ESP8266 Si4732 SSB HF Receiver.
 *
 * Keep hardware pin assignments in pin.h. This file contains values that
 * are normally changed by the builder/user.
 */

// ---------------------------------------------------------------------------
// Web Control Wi-Fi mode
// ---------------------------------------------------------------------------
// 0 = AP-first mode. The receiver starts in AP; after WiFiManager
//     successfully configures a network, later boots automatically try STA.
// 1 = Forced Station (STA) mode. If connection is unavailable, WiFiManager
//     is started automatically, then the receiver falls back to AP.
#define WEB_UI_WIFI_MODE_AP   0
#define WEB_UI_WIFI_MODE_STA  1
#define WEB_UI_WIFI_MODE      WEB_UI_WIFI_MODE_AP

// ---------------------------------------------------------------------------
// Soft-AP settings (normal/default access point)
// ---------------------------------------------------------------------------
#define AP_SSID            "Si4732-Rx"
#define AP_PASSWORD        "12345678"

// ---------------------------------------------------------------------------
// Station-mode settings
// ---------------------------------------------------------------------------
// Optional fixed credentials. In AP-first mode these are used only when a
// previously configured STA connection is being attempted. Leaving them empty
// allows WiFiManager-saved credentials to be used.
#define STA_SSID           ""
#define STA_PASSWORD       ""
#define STA_HOSTNAME       "Si4732-Rx"
#define STA_CONNECT_TIMEOUT_S  15

// WiFiManager configuration portal. In AP-first mode this is launched by the
// Web Interface "WiFi Manager" button. In forced STA mode it can launch at boot.
#define WIFI_MANAGER_AP_SSID     "Si4732-Setup"
#define WIFI_MANAGER_AP_PASSWORD "12345678"
#define WIFI_MANAGER_TIMEOUT_S   180

// ---------------------------------------------------------------------------
// Radio defaults
// ---------------------------------------------------------------------------
#define DEFAULT_VOLUME     40
#define DEFAULT_FREQUENCY  7100       // kHz
#define SPLASH_DELAY_MS    6000

// ---------------------------------------------------------------------------
// Default memory table
// ---------------------------------------------------------------------------
struct MemEntry {
  uint16_t freq;   // kHz, 0 = empty
  uint8_t  mode;   // 1 = LSB, 2 = USB, 3 = AM
};

static const MemEntry MEM_TABLE[10] = {
  {  7100, 1 },   // M1 – 40m LSB
  {  7150, 1 },   // M2
  { 14200, 2 },   // M3 – 20m USB
  { 14250, 2 },   // M4
  { 21100, 2 },   // M5 – 15m USB
  { 28400, 2 },   // M6 – 10m USB
  {  3700, 1 },   // M7 – 80m LSB
  {   810, 3 },   // M8 – MW AM
  {     0, 1 },   // M9 – empty
  {     0, 1 },   // M10 – empty
};

#endif
