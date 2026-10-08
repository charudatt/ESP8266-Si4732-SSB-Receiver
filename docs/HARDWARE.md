# Hardware Notes

## Core

- ESP8266 NodeMCU / Wemos D1 mini
- Si4732-A10 or compatible Si47XX device supporting SSB
- 1.3" SH1106 128×64 OLED
- Mechanical rotary encoder with push switch

## Wiring

| Signal | ESP8266 | Device |
|---|---|---|
| SDA | GPIO4 / D2 | Si4732 + OLED SDA |
| SCL | GPIO5 / D1 | Si4732 + OLED SCL |
| RESET | GPIO2 / D4 | Si4732 RESET |
| Encoder A | GPIO14 / D5 | Encoder A |
| Encoder B | GPIO12 / D6 | Encoder B |
| Encoder SW | GPIO13 / D7 | Encoder switch |

The encoder and switch use the ESP8266 internal pull-ups.

## Power

Use a clean 3.3 V supply and common ground.

Do not feed 5 V logic into the Si4732 I2C or control pins. The upstream PU2CLR documentation explicitly warns about this. citeturn0search1

## RF front end

The Si4732 is a DSP receiver IC, not a complete high-performance HF front end. For serious HF use, consider:

- input protection
- RF band-pass/preselector filtering
- antenna matching
- optional LNA where appropriate
- good local decoupling
- careful digital/RF ground layout

Do not assume an LNA is always beneficial; overload from strong local signals can reduce real-world performance.
