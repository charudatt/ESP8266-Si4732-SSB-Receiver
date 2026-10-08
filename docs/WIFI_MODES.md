# Web Control Wi-Fi Modes

Firmware v1.2.1 uses an **AP-first / remembered-STA** Wi-Fi design. This avoids the common problem where a newly flashed receiver cannot be reached because it is waiting for Wi-Fi credentials.

## 1. Default AP-first mode

`Config.h` contains:

```cpp
#define WEB_UI_WIFI_MODE_AP   0
#define WEB_UI_WIFI_MODE_STA  1
#define WEB_UI_WIFI_MODE      WEB_UI_WIFI_MODE_AP
```

With the default AP setting, the receiver starts as:

- SSID: `Si4732-Rx`
- Password: `12345678`
- Web interface: `http://192.168.4.1`

No router or Internet connection is required.

### Configure local Wi-Fi from AP mode

Open the receiver Web Control Interface and press **WiFi Manager**. The receiver temporarily leaves the normal Web Control server and starts the WiFiManager configuration portal using:

- SSID: `Si4732-Setup`
- Password: `12345678`

Select your local Wi-Fi network and enter its password.

When configuration succeeds:

1. The credentials are retained by WiFiManager.
2. The receiver records that STA has been successfully configured.
3. The ESP8266 restarts.
4. On the next boot it automatically attempts STA mode.

The Web Interface therefore changes from the standalone AP to your normal network without requiring a firmware rebuild.

## 2. If the local network is unavailable

If a previously configured network cannot be reached, the receiver waits only for `STA_CONNECT_TIMEOUT_S`, then starts the normal `Si4732-Rx` AP. The Web Control Interface remains available at `http://192.168.4.1`.

The next boot will try the remembered STA network again.

## 3. Forced STA mode

If desired, select:

```cpp
#define WEB_UI_WIFI_MODE      WEB_UI_WIFI_MODE_STA
```

In this mode the receiver prefers STA from startup. It can use fixed `STA_SSID`/`STA_PASSWORD` values or WiFiManager-saved credentials. If no usable credentials exist, WiFiManager starts automatically. If that process fails or times out, the receiver falls back to AP mode.

## 4. Config.h settings

```cpp
#define AP_SSID            "Si4732-Rx"
#define AP_PASSWORD        "12345678"

#define STA_SSID           ""
#define STA_PASSWORD       ""
#define STA_HOSTNAME       "Si4732-Rx"
#define STA_CONNECT_TIMEOUT_S  15

#define WIFI_MANAGER_AP_SSID     "Si4732-Setup"
#define WIFI_MANAGER_AP_PASSWORD "12345678"
#define WIFI_MANAGER_TIMEOUT_S   180
```

For normal operation, leaving `STA_SSID` and `STA_PASSWORD` empty is recommended. Use the Web Interface **WiFi Manager** button instead.

## 5. Web Interface

The Web Control Interface contains a **WiFi Manager** button below the bandwidth selector. It is available in both AP and STA operation so the operator can reconfigure the network when required. Starting WiFi Manager temporarily suspends the normal receiver Web Control page. After successful configuration the receiver restarts and uses STA automatically.

## 6. OLED

The frozen OLED layout is unchanged. Row 6 shows the active Web Interface IP address. SSID, RSSI and SNR are not displayed on the OLED.
