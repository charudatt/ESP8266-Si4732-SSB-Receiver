# Web API

The receiver exposes a small HTTP control API on the Web Control Interface. The base address depends on Wi-Fi mode:

- **AP mode:** `http://192.168.4.1`
- **STA mode:** the IP address shown on the OLED and in the Web Interface status panel

## Status

`GET /status`

Example response:

```json
{
  "freq": 14200,
  "mode": "LSB",
  "vol": 47,
  "step": 1,
  "rssi": 35,
  "snr": 14,
  "muted": false,
  "bfo": 0,
  "bfoOn": false,
  "bwSSBIdx": 2,
  "bwAMIdx": 4,
  "version": "1.2.1",
  "si4732": true,
  "wifiMode": "AP",
  "ip": "192.168.4.1",
  "uptime": "0:12:34"
}
```

## Command endpoint

`GET /cmd?a=<action>&v=<value>`

### Tuning

```text
/cmd?a=tune&v=1
/cmd?a=tune&v=-1
/cmd?a=freq&v=14200
/cmd?a=step&v=5
```

### Mode

```text
/cmd?a=mode&v=LSB
/cmd?a=mode&v=USB
/cmd?a=mode&v=AM
/cmd?a=mode&v=FM
```

### Volume and mute

```text
/cmd?a=vol&v=2
/cmd?a=volset&v=47
/cmd?a=mute&v=toggle
```

### BFO

```text
/cmd?a=bfo&v=toggle
/cmd?a=bfo&v=10
/cmd?a=bfo&v=-10
```

BFO commands are accepted only in LSB/USB.

### BFO absolute setting

```text
/cmd?a=bfoSet&v=250
/cmd?a=bfoSet&v=-500
```

Sets the SSB BFO directly in the range -16000 to +16000 Hz. The Web Interface slider uses 10 Hz steps and enables BFO automatically.

### Bandwidth

SSB:

```text
/cmd?a=bwssb&v=0
```

Indexes:

| Index | Bandwidth |
|---:|---:|
| 0 | 1.2 kHz |
| 1 | 2.2 kHz |
| 2 | 3.0 kHz |
| 3 | 4.0 kHz |
| 4 | 0.5 kHz |
| 5 | 1.0 kHz |

AM:

| Index | Bandwidth |
|---:|---:|
| 0 | 6.0 kHz |
| 1 | 4.0 kHz |
| 2 | 3.0 kHz |
| 3 | 2.0 kHz |
| 4 | 1.0 kHz |
| 5 | 1.8 kHz |
| 6 | 2.5 kHz |

### Memory

```text
/cmd?a=mem_store&v=0
/cmd?a=mem_recall&v=0
```

Slots are zero-based in the API and correspond to M1–M10 in firmware storage. The Web Control Interface also exposes M1–M10 STORE and RECALL controls, and the memory dropdown shows each stored frequency and mode (or Empty).


## WiFi Manager

`GET /wifi-manager`

Starts the WiFiManager configuration portal. This endpoint is exposed by the **WiFi Manager** button in the Web Control Interface. The receiver temporarily stops the normal Web Control server while WiFiManager is active.

- Normal AP mode: connect to the `Si4732-Setup` configuration network if prompted.
- After successful configuration, the receiver stores the STA-configured state and restarts.
- Subsequent boots automatically try the saved local Wi-Fi network in AP-first mode.
- If configuration times out or fails, the receiver returns to the normal `Si4732-Rx` AP.
