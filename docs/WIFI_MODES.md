# Wi-Fi Modes

## Default: AP mode

The receiver normally starts its own access point. Connect a phone/tablet/PC to the receiver's AP and open `http://192.168.4.1`.

## STA mode

STA mode makes the ESP8266 join an existing Wi-Fi network. This is useful when the Web Control Interface is to be used on a larger screen or computer already connected to the local network.

The selected mode is configured in `Config.h`. The project keeps AP as the default so the receiver remains directly accessible without a router.

## WiFiManager

Use the Web Interface's **WiFi Manager** control when the receiver needs to be connected to a local network. Select the desired network and enter its password. After successful configuration, the credentials are retained for subsequent starts.

If the saved network is unavailable, the receiver falls back to its own AP so that the Web Interface remains accessible.

## Recommended user workflow

1. Power the receiver.
2. Use AP mode and connect to the receiver if no local Wi-Fi credentials have been configured.
3. Open the Web Interface at `192.168.4.1`.
4. Select **WiFi Manager**.
5. Select the local Wi-Fi network and enter its password.
6. After successful configuration, use the IP shown on the OLED to access the receiver from the larger screen.
7. If the local network is unavailable later, use the receiver AP again.
