/*
  ESP8266 + Si4732 SSB HF Receiver
  --------------------------------
  Display : 1.3" OLED SH1106 128x64 (U8x8)
  Library : PU2CLR SI4735
  Controls: Rotary Encoder + configurable AP/STA Web UI

  Pins (see pin.h):
    I2C  SCL=D1 (GPIO5), SDA=D2 (GPIO4)
    Si4732 RST = D4 (GPIO2)
    Encoder A=D5 (GPIO14), B=D6 (GPIO12), SW=D7 (GPIO13)

  Libraries required:
    - PU2CLR SI4735
    - U8g2 (for U8x8)
    - ESP8266 board package
    - WiFiManager (tzapu)

  Author: generated for user request
*/

#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <EEPROM.h>
#include <SI4735.h>
#include <U8x8lib.h>

#include "version.h"
#include "pin.h"
#include "Rotary.h"

// SSB patch – choose one that matches your library version
// Most recent PU2CLR library ships patch_init.h or patch_ssb_compressed.h
#include <patch_init.h>          // fallback; change to patch_ssb_compressed.h if needed

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------
#define FM_BAND_TYPE  0
#define MW_BAND_TYPE  1
#define SW_BAND_TYPE  2
#define LW_BAND_TYPE  3

// Sideband selection values required by SI4735::setSSB()
// (library does not define these symbols – official examples define them locally)
#define LSB  1
#define USB  2

#define MODE_FM   0
#define MODE_LSB  1
#define MODE_USB  2
#define MODE_AM   3

#define EEPROM_SIZE      256
#define APP_ID           0x53   // 'S'
#define STORE_AFTER_MS   8000   // save to EEPROM after inactivity
#define NUM_MEM          10     // memory channels 1..10
#define MEM_EEPROM_BASE  20     // start address for mem slots (3 bytes each)
#define WIFI_STA_EEPROM_ADDR 11
#define WIFI_STA_CONFIGURED  0xA5
// DEFAULT_VOLUME, DEFAULT_FREQUENCY, Wi-Fi settings and SPLASH_DELAY_MS → Config.h

// ---------------------------------------------------------------------------
// Hardware objects
// ---------------------------------------------------------------------------
// SH1106 128x64 – explicit pins help on ESP8266
// If still blank after upload, comment the HW line and uncomment the SW_I2C line.
U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(U8X8_PIN_NONE, /*clock=*/ PIN_SCL, /*data=*/ PIN_SDA);
// U8X8_SH1106_128X64_NONAME_SW_I2C u8x8(/*clock=*/ PIN_SCL, /*data=*/ PIN_SDA, /*reset=*/ U8X8_PIN_NONE);

SI4735  rx;
Rotary  encoder(PIN_ENC_A, PIN_ENC_B);
ESP8266WebServer server(80);

// ---------------------------------------------------------------------------
// Global state
// ---------------------------------------------------------------------------
volatile int encoderCount = 0;

uint16_t currentFrequency = DEFAULT_FREQUENCY;
uint8_t  currentMode      = MODE_LSB;   // start in LSB
int16_t  currentBFO       = 0;
uint8_t  volume           = DEFAULT_VOLUME;
bool     ssbLoaded        = false;
bool     bfoOn            = false;

uint8_t  currentStep      = 1;          // kHz (or 10 Hz when BFO)
uint8_t  bwIdxSSB         = 4;          // 3.0 kHz default
uint8_t  bwIdxAM          = 4;

const char* modeStr[] = { "FM ", "LSB", "USB", "AM " };

// Simple band table (expand as needed)
struct Band {
  const char* name;
  uint8_t     type;
  uint16_t    minFreq;
  uint16_t    maxFreq;
  uint16_t    defaultFreq;
  uint8_t     step;           // default step index
};

Band band[] = {
  { "MW ", MW_BAND_TYPE,  520, 1710,  810, 1 },
  { "80m", SW_BAND_TYPE, 3500, 4000, 3700, 0 },
  { "40m", SW_BAND_TYPE, 7000, 7300, DEFAULT_FREQUENCY, 0 },
  { "20m", SW_BAND_TYPE,14000,14350,14200, 0 },
  { "15m", SW_BAND_TYPE,21000,21450,21100, 0 },
  { "10m", SW_BAND_TYPE,28000,29700,28400, 0 },
  { "ALL", SW_BAND_TYPE, 150, 30000,15000, 1 }
};
const int lastBand = sizeof(band) / sizeof(Band) - 1;
int bandIdx = 2;   // start on 40 m

// Step tables
uint8_t stepIdx = 0;

// Bandwidth tables (index used by SI4735 library)
// SSB: 0=1.2, 1=2.2, 2=3.0, 3=4.0, 4=0.5, 5=1.0
const char* bwSSB[] = { "1.2", "2.2", "3.0", "4.0", "0.5", "1.0" };
// AM : 0=6, 1=4, 2=3, 3=2, 4=1, 5=1.8, 6=2.5
const char* bwAM[]  = { "6.0", "4.0", "3.0", "2.0", "1.0", "1.8", "2.5" };

// Timing
unsigned long lastActivity = 0;
unsigned long lastRSSI     = 0;
bool needSave = false;
bool muted = false;   // audio mute state

// Wi-Fi runtime state
bool webWiFiIsAP = true;
IPAddress webIPAddr(192, 168, 4, 1);
bool si4732Detected = false;

// Memory channels (freq kHz, mode) – empty slot = freq 0
uint16_t memFreq[NUM_MEM];
uint8_t  memMode[NUM_MEM];

// ---------------------------------------------------------------------------
// Forward declarations
// ---------------------------------------------------------------------------
void showStatus();
void useBand();
void loadSSB();
void switchMode(uint8_t newMode);
void handleRoot();
void handleCmd();
void handleStatus();
void handleMem();
void saveSettings();
void loadSettings();
void loadMemories();
void saveMemorySlot(uint8_t slot);
void recallMemory(uint8_t slot);
void startWebWiFi();
bool startWiFiManagerPortal();
void startSoftAP();
void registerWebRoutes();

// ---------------------------------------------------------------------------
// ISR
// ---------------------------------------------------------------------------
ICACHE_RAM_ATTR void rotaryEncoder() {
  uint8_t result = encoder.process();
  if (result == DIR_CW)  encoderCount++;
  if (result == DIR_CCW) encoderCount--;
}

// ---------------------------------------------------------------------------
// Setup
// ---------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println(F("\nESP8266 Si4732 SSB Rx starting..."));

  pinMode(PIN_ENC_SW, INPUT_PULLUP);
  encoder.begin(true);   // enable pull-ups

  // ---- I2C + OLED first so user sees something immediately ----
  Wire.begin(PIN_SDA, PIN_SCL);   // D2=SDA, D1=SCL
  Wire.setClock(100000);

  u8x8.begin();
  u8x8.setPowerSave(0);
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.clear();
  u8x8.drawString(0, 0, "Si4732 SSB Rx");
  u8x8.drawString(0, 1, APP_VERSION_LABEL);
  u8x8.drawString(0, 2, "Starting...");
  u8x8.drawString(0, 4, "Init radio...");

  // Radio
  Wire.beginTransmission(0x11);
  si4732Detected = (Wire.endTransmission() == 0);
  rx.setI2CFastModeCustom(100000);
  rx.getDeviceI2CAddress(PIN_SI4732_RST);   // auto-detect address
  rx.setup(PIN_SI4732_RST, MW_BAND_TYPE);
  delay(300);

  // EEPROM
  EEPROM.begin(EEPROM_SIZE);
  if (EEPROM.read(0) == APP_ID) {
    loadSettings();
  } else {
    volume = DEFAULT_VOLUME;
    rx.setVolume(volume);
  }
  loadMemories();

  // Start in selected band / mode
  useBand();
  if (currentMode == MODE_LSB || currentMode == MODE_USB) {
    loadSSB();
  }

  // Encoder interrupts
  attachInterrupt(digitalPinToInterrupt(PIN_ENC_A), rotaryEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_ENC_B), rotaryEncoder, CHANGE);

  // ---- Web control Wi-Fi ----
  // Default is Soft-AP. STA mode can be selected in Config.h; if credentials
  // are missing or unavailable, WiFiManager provides a captive portal to scan
  // for and enter the user's Wi-Fi credentials.
  startWebWiFi();

  Serial.println(F("================================="));
  Serial.print(F("  Wi-Fi mode: ")); Serial.println(webWiFiIsAP ? F("AP") : F("STA"));
  if (webWiFiIsAP) {
    Serial.print(F("  AP SSID   : ")); Serial.println(AP_SSID);
    Serial.print(F("  Password  : ")); Serial.println(AP_PASSWORD);
  } else {
    Serial.print(F("  SSID      : ")); Serial.println(WiFi.SSID());
  }
  Serial.print(F("  IP        : ")); Serial.println(webIPAddr);
  Serial.println(F("================================="));

  // Splash: active Web Interface connection details.
  // SSID is intentionally not shown on the frozen OLED status display.
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.clear();
  u8x8.drawString(0, 0, webWiFiIsAP ? "=== WiFi AP ===" : "=== WiFi STA ===");
  char line[17];
  if (webWiFiIsAP) {
    snprintf(line, sizeof(line), "Pass:%s", AP_PASSWORD);
    u8x8.drawString(0, 3, line);
  } else {
    u8x8.drawString(0, 3, "Connected");
  }
  snprintf(line, sizeof(line), "IP: %s", webIPAddr.toString().c_str());
  u8x8.drawString(0, 5, line);
  u8x8.drawString(0, 7, "Open browser");
  delay(SPLASH_DELAY_MS);

  registerWebRoutes();
  server.begin();

  lastActivity = millis();
  showStatus();
  Serial.println(F("Ready."));
}

// ---------------------------------------------------------------------------
// Main loop
// ---------------------------------------------------------------------------
void loop() {
  server.handleClient();

  // ----- Encoder rotation -----
  if (encoderCount != 0) {
    int steps = encoderCount;
    encoderCount = 0;
    lastActivity = millis();
    needSave = true;

    if (bfoOn) {
      currentBFO += steps * 10;          // 10 Hz
      if (currentBFO > 16000)  currentBFO = 16000;
      if (currentBFO < -16000) currentBFO = -16000;
      rx.setSSBBfo(currentBFO);
    } else {
      int16_t delta = steps * (int16_t)currentStep;
      int32_t newFreq = (int32_t)currentFrequency + delta;
      if (newFreq < band[bandIdx].minFreq) newFreq = band[bandIdx].minFreq;
      if (newFreq > band[bandIdx].maxFreq) newFreq = band[bandIdx].maxFreq;
      currentFrequency = (uint16_t)newFreq;
      rx.setFrequency(currentFrequency);
    }
    showStatus();
  }

  // ----- Encoder button -----
  static unsigned long btnDown = 0;
  static bool btnHandled = false;
  if (digitalRead(PIN_ENC_SW) == LOW) {
    if (btnDown == 0) {
      btnDown = millis();
      btnHandled = false;
    } else if (!btnHandled && (millis() - btnDown > 600)) {
      // Long press → toggle BFO
      btnHandled = true;
      bfoOn = !bfoOn;
      lastActivity = millis();
      showStatus();
    }
  } else {
    if (btnDown != 0 && !btnHandled && (millis() - btnDown < 500)) {
      // Short press → cycle mode
      uint8_t next = currentMode + 1;
      if (next > MODE_AM) next = MODE_LSB;   // skip FM for HF-focused build
      switchMode(next);
      lastActivity = millis();
      needSave = true;
      showStatus();
    }
    btnDown = 0;
    btnHandled = false;
  }

  // ----- Auto-save -----
  if (needSave && (millis() - lastActivity > STORE_AFTER_MS)) {
    saveSettings();
    needSave = false;
  }
}

// ---------------------------------------------------------------------------
// Radio helpers
// ---------------------------------------------------------------------------
void loadSSB() {
  if (ssbLoaded) return;

  // Classic PU2CLR sequence (works with all recent library versions)
  rx.reset();
  rx.queryLibraryId();
  rx.patchPowerUp();
  delay(50);
  rx.setI2CFastModeCustom(500000);          // speed up patch transfer
  rx.downloadPatch(ssb_patch_content, sizeof(ssb_patch_content));
  rx.setI2CFastModeCustom(100000);          // back to normal
  delay(50);

  // Configure SSB mode defaults
  // AUDIOBW, SBCUTFLT, AVC_DIVIDER, AVCEN, SMUTESEL, DSP_AFCDIS
  rx.setSSBConfig(bwIdxSSB, 1, 0, 1, 0, 1);

  ssbLoaded = true;
}

void useBand() {
  Band& b = band[bandIdx];
  currentFrequency = b.defaultFreq;

  if (b.type == FM_BAND_TYPE) {
    currentMode = MODE_FM;
    rx.setFM(b.minFreq, b.maxFreq, currentFrequency, 10);
    ssbLoaded = false;
  } else {
    // Prefer SSB on ham bands
    if (currentMode == MODE_FM) currentMode = MODE_LSB;
    if (currentMode == MODE_LSB || currentMode == MODE_USB) {
      loadSSB();
      rx.setSSB(b.minFreq, b.maxFreq, currentFrequency, currentStep,
                currentMode == MODE_LSB ? LSB : USB);
      rx.setSSBAutomaticVolumeControl(1);
      rx.setSSBAudioBandwidth(bwIdxSSB);
      if (bwIdxSSB == 0 || bwIdxSSB == 4 || bwIdxSSB == 5)
        rx.setSSBSidebandCutoffFilter(0);
      else
        rx.setSSBSidebandCutoffFilter(1);
    } else {
      rx.setAM(b.minFreq, b.maxFreq, currentFrequency, currentStep);
      rx.setBandwidth(bwIdxAM, 1);
    }
  }
  rx.setVolume(volume);
  currentBFO = 0;
  bfoOn = false;
}

void switchMode(uint8_t newMode) {
  currentMode = newMode;
  Band& b = band[bandIdx];

  if (newMode == MODE_LSB || newMode == MODE_USB) {
    loadSSB();
    rx.setSSB(b.minFreq, b.maxFreq, currentFrequency, currentStep,
              newMode == MODE_LSB ? LSB : USB);
    rx.setSSBAutomaticVolumeControl(1);
    rx.setSSBAudioBandwidth(bwIdxSSB);
    rx.setSSBBfo(currentBFO);
  } else if (newMode == MODE_AM) {
    rx.setAM(b.minFreq, b.maxFreq, currentFrequency, currentStep);
    rx.setBandwidth(bwIdxAM, 1);
    ssbLoaded = false;
  } else { // FM
    rx.setFM(b.minFreq, b.maxFreq, currentFrequency, 10);
    ssbLoaded = false;
  }
  bfoOn = false;
}

void startSoftAP() {
  webWiFiIsAP = true;
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  delay(100);
  webIPAddr = WiFi.softAPIP();
  Serial.print(F("Soft-AP started. IP: "));
  Serial.println(webIPAddr);
}

bool connectSavedOrConfiguredSTA() {
  WiFi.mode(WIFI_STA);
  WiFi.hostname(STA_HOSTNAME);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(false);

  String savedSSID = WiFi.SSID();
  const bool configCredentials = strlen(STA_SSID) > 0;
  const bool savedCredentials = savedSSID.length() > 0;

  if (!configCredentials && !savedCredentials) {
    Serial.println(F("No STA credentials are available."));
    return false;
  }

  if (configCredentials) {
    Serial.print(F("Connecting to configured STA SSID: "));
    Serial.println(STA_SSID);
    WiFi.begin(STA_SSID, STA_PASSWORD);
  } else {
    Serial.print(F("Connecting to saved STA SSID: "));
    Serial.println(savedSSID);
    WiFi.begin();
  }

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < (STA_CONNECT_TIMEOUT_S * 1000UL)) {
    delay(250);
    Serial.print('.');
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    webWiFiIsAP = false;
    webIPAddr = WiFi.localIP();
    Serial.print(F("STA connected. IP: "));
    Serial.println(webIPAddr);
    return true;
  }

  Serial.println(F("STA connection failed."));
  return false;
}

bool startWiFiManagerPortal() {
  Serial.println(F("Starting WiFiManager configuration portal..."));

  server.stop();
  WiFi.disconnect(false);
  delay(100);

  WiFiManager wm;
  wm.setDebugOutput(true);
  wm.setHostname(STA_HOSTNAME);
  wm.setConnectTimeout(STA_CONNECT_TIMEOUT_S);
  wm.setConfigPortalTimeout(WIFI_MANAGER_TIMEOUT_S);

  // startConfigPortal() is deliberately used here because the receiver is
  // already running in its normal AP mode and the operator explicitly asked
  // for WiFi configuration from the Web Interface.
  bool connected = wm.startConfigPortal(WIFI_MANAGER_AP_SSID, WIFI_MANAGER_AP_PASSWORD);

  if (connected) {
    EEPROM.write(WIFI_STA_EEPROM_ADDR, WIFI_STA_CONFIGURED);
    EEPROM.commit();
    Serial.println(F("WiFi credentials accepted. STA mode will be used on next boot."));
    return true;
  }

  Serial.println(F("WiFiManager timed out/cancelled. Staying in Soft-AP mode."));
  startSoftAP();
  return false;
}

void startWebWiFi() {
#if WEB_UI_WIFI_MODE == WEB_UI_WIFI_MODE_AP
  // AP is the safe/default startup mode. If WiFiManager has previously
  // configured a network, try STA automatically; otherwise remain AP.
  bool staRemembered = (EEPROM.read(WIFI_STA_EEPROM_ADDR) == WIFI_STA_CONFIGURED);
  if (staRemembered) {
    Serial.println(F("Previously configured WiFi found; trying STA first."));
    if (connectSavedOrConfiguredSTA()) return;
    Serial.println(F("STA unavailable; falling back to normal Soft-AP."));
  }
  startSoftAP();
#else
  // Forced STA mode selected in Config.h. If no credentials are available,
  // open WiFiManager immediately. A failed portal falls back to AP.
  if (connectSavedOrConfiguredSTA()) return;
  if (startWiFiManagerPortal()) {
    ESP.restart();
  }
#endif
}

void registerWebRoutes() {
  server.on("/",        handleRoot);
  server.on("/cmd",     handleCmd);
  server.on("/status",  handleStatus);
  server.on("/mem",     handleMem);
  server.on("/wifi-manager", []() {
    server.send(200, "text/plain", "Starting WiFi Manager. Connect to Si4732-Setup if required.");
    delay(100);
    bool connected = startWiFiManagerPortal();
    if (connected) {
      delay(500);
      ESP.restart();
    } else {
      registerWebRoutes();
      server.begin();
    }
  });
}

// ---------------------------------------------------------------------------
// Display
// ---------------------------------------------------------------------------
void showStatus() {
  char buf[24];

  // Avoid full-screen clear() here. Redrawing fixed-width fields prevents the
  // visible OLED blink that occurs when the entire display is cleared on every
  // encoder click.
  // Row 0-1: frequency in the large 2x2 font.
  // Five frequency digits occupy 10 normal character columns.
  u8x8.setFont(u8x8_font_px437wyse700b_2x2_r);
  u8x8.drawString(0, 0, "          ");
  snprintf(buf, sizeof(buf), "%5u", currentFrequency);
  u8x8.drawString(0, 0, buf);

  // MODE uses the normal font so it can sit on the same line as the
  // large frequency, with exactly one character column between them.
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.drawString(10, 0, "      ");
  u8x8.drawString(10, 1, "      ");
  u8x8.drawString(11, 0, modeStr[currentMode]);

  // Remaining status rows use the normal 8x8 font.
  // Band is intentionally not shown on the OLED.
  snprintf(buf, sizeof(buf), "Step:%3u  V:%2u", currentStep, muted ? 0 : volume);
  // The 2x2 frequency font occupies rows 0 and 1. Start normal text on row 2.
  u8x8.drawString(0, 2, buf);

  const char* currentBW = (currentMode == MODE_LSB || currentMode == MODE_USB)
                              ? bwSSB[bwIdxSSB]
                              : (currentMode == MODE_AM ? bwAM[bwIdxAM] : "--");
  if (bfoOn && (currentMode == MODE_LSB || currentMode == MODE_USB)) {
    snprintf(buf, sizeof(buf), "BFO:%+6d BW:%s", currentBFO, currentBW);
  } else {
    snprintf(buf, sizeof(buf), "BFO: OFF BW:%s", currentBW);
  }
  // Row 3: BFO + bandwidth. Explicitly clear first so no previous-screen
  // characters remain.
  u8x8.drawString(0, 3, "                ");
  u8x8.drawString(0, 3, buf);

  // Rows 4 and 5 are intentionally blank. Clear them explicitly.
  u8x8.drawString(0, 4, "                ");
  u8x8.drawString(0, 5, "                ");

  // Row 6: IP address.
  u8x8.drawString(0, 6, "                ");
  u8x8.drawString(0, 6, webIPAddr.toString().c_str());

  // Row 7: firmware/status.
  u8x8.drawString(0, 7, "                ");
  snprintf(buf, sizeof(buf), "v%s %s", APP_VERSION, muted ? "MUTE" : "READY");
  u8x8.drawString(0, 7, buf);
}

// ---------------------------------------------------------------------------
// EEPROM
// ---------------------------------------------------------------------------
void saveSettings() {
  EEPROM.write(0, APP_ID);
  EEPROM.write(1, volume);
  EEPROM.write(2, bandIdx);
  EEPROM.write(3, currentMode);
  EEPROM.write(4, currentFrequency >> 8);
  EEPROM.write(5, currentFrequency & 0xFF);
  EEPROM.write(6, (currentBFO >> 8) & 0xFF);
  EEPROM.write(7, currentBFO & 0xFF);
  EEPROM.write(8, currentStep);
  EEPROM.write(9, bwIdxSSB);
  EEPROM.write(10, bwIdxAM);
  EEPROM.commit();
  Serial.println(F("Settings saved"));
}

void loadSettings() {
  volume           = constrain(EEPROM.read(1), 0, 63);
  bandIdx          = constrain(EEPROM.read(2), 0, lastBand);
  currentMode      = constrain(EEPROM.read(3), MODE_LSB, MODE_AM);
  currentFrequency = (EEPROM.read(4) << 8) | EEPROM.read(5);
  currentBFO       = (int16_t)((EEPROM.read(6) << 8) | EEPROM.read(7));
  currentStep      = EEPROM.read(8);
  if (currentStep == 0 || currentStep > 100) currentStep = 1;
  bwIdxSSB         = constrain(EEPROM.read(9), 0, 5);
  bwIdxAM          = constrain(EEPROM.read(10), 0, 6);

  // Reject corrupt/out-of-range EEPROM frequency data.
  if (currentFrequency < 150 || currentFrequency > 30000) {
    currentFrequency = band[bandIdx].defaultFreq;
  }

  if (currentBFO < -16000 || currentBFO > 16000) {
    currentBFO = 0;
  }

  rx.setVolume(volume);
}

void loadMemories() {
  for (uint8_t i = 0; i < NUM_MEM; i++) {
    int addr = MEM_EEPROM_BASE + i * 3;
    uint16_t ef = (EEPROM.read(addr) << 8) | EEPROM.read(addr + 1);
    uint8_t  em = EEPROM.read(addr + 2);
    // Use Config.h MEM_TABLE when EEPROM slot is blank/empty
    if (ef == 0 || ef == 0xFFFF) {
      memFreq[i] = MEM_TABLE[i].freq;
      memMode[i] = MEM_TABLE[i].mode;
    } else {
      memFreq[i] = ef;
      memMode[i] = em;
    }
    if (memMode[i] < MODE_LSB || memMode[i] > MODE_AM)
      memMode[i] = MODE_LSB;
  }
}

void saveMemorySlot(uint8_t slot) {
  if (slot >= NUM_MEM) return;
  memFreq[slot] = currentFrequency;
  memMode[slot] = currentMode;
  int addr = MEM_EEPROM_BASE + slot * 3;
  EEPROM.write(addr,     memFreq[slot] >> 8);
  EEPROM.write(addr + 1, memFreq[slot] & 0xFF);
  EEPROM.write(addr + 2, memMode[slot]);
  EEPROM.commit();
  Serial.print(F("MEM")); Serial.print(slot + 1);
  Serial.print(F(" saved: ")); Serial.println(memFreq[slot]);
}

void recallMemory(uint8_t slot) {
  if (slot >= NUM_MEM) return;
  if (memFreq[slot] == 0) return;  // empty
  currentFrequency = memFreq[slot];
  currentMode      = memMode[slot];
  // pick a band that contains this frequency
  for (int i = 0; i <= lastBand; i++) {
    if (currentFrequency >= band[i].minFreq && currentFrequency <= band[i].maxFreq) {
      bandIdx = i;
      break;
    }
  }
  switchMode(currentMode);
  rx.setFrequency(currentFrequency);
  needSave = true;
  lastActivity = millis();
  showStatus();
}

// ---------------------------------------------------------------------------
// Web UI
// ---------------------------------------------------------------------------
const char webPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>ESP8266 Si4732 SSB Receiver</title>
<style>
:root{--blue:#1677d2;--blue2:#0d5ea8;--green:#18a957;--bg:#f4f7fa;--border:#c9d2dc;--text:#17212b}
*{box-sizing:border-box}
body{margin:0;background:var(--bg);font-family:Arial,Helvetica,sans-serif;color:var(--text)}
.app{max-width:900px;margin:18px auto;background:#fff;border:1px solid #b8c2cc;border-radius:8px;box-shadow:0 2px 10px #0002;overflow:hidden}
.header{background:linear-gradient(135deg,#1b477b,#123c6b);color:#fff;padding:16px 20px;font-size:24px;font-weight:700;display:flex;justify-content:space-between;align-items:center}
.header span{font-size:15px;font-weight:600;opacity:.9}
.content{padding:20px 24px}
.grid3{display:grid;grid-template-columns:1.7fr .9fr .9fr;gap:18px;align-items:end}
label{display:block;font-weight:700;margin:0 0 7px;font-size:16px}
input,select{width:100%;height:48px;border:1px solid var(--border);border-radius:6px;background:#fff;color:#111;font-size:20px;padding:8px 12px}
button{border:0;border-radius:6px;background:var(--blue);color:#fff;font-weight:700;font-size:17px;padding:12px 18px;cursor:pointer;min-height:48px}
button:hover{background:var(--blue2)}
button.active{background:var(--green)}
button:disabled{opacity:.45;cursor:not-allowed}
.tune{display:grid;grid-template-columns:repeat(4,1fr);gap:6px;margin:14px 0}
.mode{display:grid;grid-template-columns:repeat(4,1fr);gap:8px;margin:14px 0 22px}
.control{display:grid;grid-template-columns:145px 1fr 92px;gap:14px;align-items:center;margin:16px 0}
.control .value{height:48px;border:1px solid var(--border);border-radius:6px;display:flex;align-items:center;justify-content:center;font-size:18px;background:#fff}
input[type=range]{height:auto;padding:0;accent-color:var(--blue)}
.bfo{display:grid;grid-template-columns:105px 1fr 92px;gap:14px;align-items:center}
.toggle{border-radius:24px;padding:8px 15px;min-height:42px}
.toggle.on{background:var(--green)}
.subrow{display:flex;justify-content:space-between;color:#444;font-size:14px;margin-top:-8px;padding-left:160px}
.bandwidth{display:grid;grid-template-columns:145px 280px 1fr;gap:14px;align-items:center;margin:18px 0}
hr{border:0;border-top:1px solid #aeb8c2;margin:22px 0}
.status{background:#eaf7ee;border:1px solid #b6dec4;border-radius:6px;padding:14px 18px;display:grid;grid-template-columns:140px 1fr;gap:10px 14px;font-size:17px}
.meter{height:16px;background:#d7dee5;border-radius:3px;overflow:hidden;margin-top:2px}.meter i{display:block;height:100%;background:var(--green);width:0}
.statusgrid{display:grid;grid-template-columns:1fr 1fr;gap:10px 28px}.full{grid-column:1/-1}
.memory{display:grid;grid-template-columns:145px 1fr 120px 120px;gap:14px;align-items:center;margin:18px 0}.memory select{height:48px}.memory .meminfo{font-size:15px;color:#56616b}.wifiAction{display:flex;align-items:center;gap:14px;margin:16px 0}.wifiAction button{min-width:160px}.wifiAction span{font-size:14px;color:#56616b}.small{font-size:13px;color:#56616b;margin-top:14px;text-align:center}
@media(max-width:700px){.app{margin:0;border-radius:0}.content{padding:14px}.grid3,.control,.bfo,.bandwidth,.memory,.statusgrid{grid-template-columns:1fr}.subrow{padding-left:0}.tune,.mode{grid-template-columns:repeat(2,1fr)}.header{font-size:20px}}
</style>
</head>
<body>
<div class="app">
  <div class="header">ESP8266 Si4732 SSB Receiver <span id="ver">v1.2.2</span></div>
  <div class="content">
    <div class="grid3">
      <div><label>Frequency (kHz)</label><input id="freq" type="number" min="150" max="30000" step="1" value="14200" onchange="setFreq()"></div>
      <div><label>Mode</label><select id="mode" onchange="setMode(this.value)"><option>LSB</option><option>USB</option><option>AM</option><option>FM</option></select></div>
      <div><label>Step (kHz)</label><select id="step" onchange="setStep(this.value)"><option>1</option><option>5</option><option>9</option><option>10</option><option>50</option><option>100</option></select></div>
    </div>

    <div class="tune"><button onclick="tune(-1)">&lt;</button><button onclick="tune(-10)">&lt;&lt;</button><button onclick="tune(10)">&gt;&gt;</button><button onclick="tune(1)">&gt;</button></div>
    <div class="mode"><button id="am" onclick="setMode('AM')">AM</button><button id="fm" onclick="setMode('FM')">FM</button><button id="lsb" onclick="setMode('LSB')">LSB</button><button id="usb" onclick="setMode('USB')">USB</button></div>

    <div class="control"><label>Volume</label><input id="vol" type="range" min="0" max="63" step="1" value="47" oninput="showVol()" onchange="setVol()"><div class="value" id="volv">47</div></div>
    <div style="display:flex;justify-content:flex-end;margin:-6px 0 16px"><button id="muteBtn" class="toggle" onclick="toggleMute()">MUTE OFF</button></div>

    <div class="bfo">
      <button id="bfoToggle" class="toggle" onclick="toggleBfo()">BFO OFF</button>
      <input id="bfo" type="range" min="-16000" max="16000" step="10" value="0" oninput="showBfo()" onchange="setBfo()">
      <div class="value" id="bfov">0</div>
    </div>
    <div class="subrow"><span>-16000</span><span>0</span><span>+16000</span></div>

    <div class="bandwidth"><label>Bandwidth (kHz)</label><select id="bw" onchange="setBw(this.value)"></select><span id="bwtype">(SSB)</span></div>

    <div class="memory">
      <label>Memory</label>
      <select id="memslot" onchange="loadMemInfo()">
        <option value="0">M1</option><option value="1">M2</option><option value="2">M3</option><option value="3">M4</option><option value="4">M5</option>
        <option value="5">M6</option><option value="6">M7</option><option value="7">M8</option><option value="8">M9</option><option value="9">M10</option>
      </select>
      <button onclick="memStore()">STORE</button>
      <button onclick="memRecall()">RECALL</button>
    </div>
    <div class="memory"><span></span><span class="meminfo" id="meminfo">M1: empty</span><span></span><span></span></div>

    <div class="wifiAction"><button onclick="openWiFiManager()">WiFi Manager</button><span id="wifiHelp">Configure this receiver to join your local Wi-Fi network.</span></div>

    <hr>
    <div class="statusgrid">
      <div class="status full">
        <strong>RSSI:</strong><span><span id="rssiText">--</span> dBm <span class="meter"><i id="rssiBar"></i></span></span>
        <strong>SNR:</strong><span><span id="snrText">--</span> dB <span class="meter"><i id="snrBar"></i></span></span>
        <strong>Si4732:</strong><span id="chip">--</span>
        <strong>WiFi:</strong><span id="wifi">--</span>
        <strong>WiFi IP:</strong><span id="ip">--</span>
        <strong>Uptime:</strong><span id="uptime">--</span>
        <strong>Mode:</strong><span id="smode">--</span>
        <strong>Frequency:</strong><span id="sfreq">--</span>
        <strong>Step:</strong><span id="sstep">--</span>
        <strong>Volume:</strong><span id="svol">--</span>
        <strong>BFO:</strong><span id="sbfo">--</span>
        <strong>BW:</strong><span id="sbw">--</span>
      </div>
    </div>
    <div class="small">Web Control Interface • ESP8266 Si4732 SSB Receiver</div>
  </div>
</div>
<script>
const SSB=['1.2','2.2','3.0','4.0','0.5','1.0'], AM=['6.0','4.0','3.0','2.0','1.0','1.8','2.5'];
function get(a,v){return fetch('/cmd?a='+encodeURIComponent(a)+'&v='+encodeURIComponent(v)).then(r=>r.text())}
function setFreq(){let v=+freq.value;if(v>=150&&v<=30000)get('freq',v).then(update)}
function tune(v){get('tune',v).then(update)}
function setMode(v){get('mode',v).then(update)}
function setStep(v){get('step',v).then(update)}
function showVol(){volv.textContent=vol.value}
function setVol(){get('volset',vol.value).then(update)}
function toggleMute(){get('mute','toggle').then(update)}
function toggleBfo(){get('bfo','toggle').then(update)}
function showBfo(){bfov.textContent=(+bfo.value>=0?'+':'')+bfo.value}
function setBfo(){get('bfoSet',bfo.value).then(update)}
function setBw(v){get(mode.value==='AM'?'bwam':'bwssb',v).then(update)}
function loadMemInfo(){fetch('/mem').then(r=>r.json()).then(a=>{let i=+memslot.value,m=a[i];meminfo.textContent='M'+(i+1)+': '+(m.freq?m.freq+' kHz '+m.mode:'empty')})}
function memStore(){let i=+memslot.value;get('mem_store',i).then(t=>{alert(t);loadMemInfo()})}
function memRecall(){let i=+memslot.value;get('mem_recall',i).then(t=>{alert(t);update();loadMemInfo()})}
function openWiFiManager(){if(!confirm('Start WiFi Manager? The receiver will temporarily leave the Web Control page.'))return;fetch('/wifi-manager').then(()=>{document.body.innerHTML='<div style=\"font-family:Arial;padding:30px\"><h2>WiFi Manager started</h2><p>Connect to <b>Si4732-Setup</b> if prompted, configure your Wi-Fi, then wait for the receiver to restart.</p></div>'}).catch(()=>{})}
function buildBw(d){if(d.mode==='FM'){bw.innerHTML='<option value=0>--</option>';bw.disabled=true;return}bw.disabled=false;let a=(d.mode==='AM')?AM:SSB;bw.innerHTML='';a.forEach((x,i)=>{let o=document.createElement('option');o.value=i;o.textContent=x;bw.appendChild(o)});bw.value=(d.mode==='AM'?d.bwAMIdx:d.bwSSBIdx)}
function update(){fetch('/status').then(r=>r.json()).then(d=>{
  freq.value=d.freq;mode.value=d.mode;step.value=d.step;vol.value=d.muted?0:d.vol;volv.textContent=d.muted?0:d.vol;bfo.value=d.bfo;showBfo();
  buildBw(d); bwtype.textContent=d.mode==='FM'?'(FM)':(d.mode==='AM'?'(AM)':'(SSB)');
  muteBtn.textContent=d.muted?'MUTE ON':'MUTE OFF';muteBtn.className='toggle'+(d.muted?' on':'');
  bfoToggle.textContent=d.bfoOn?'BFO ON':'BFO OFF';bfoToggle.className='toggle'+(d.bfoOn?' on':'');bfo.disabled=!(d.mode==='LSB'||d.mode==='USB');
  ['am','fm','lsb','usb'].forEach(x=>document.getElementById(x).classList.remove('active'));if(d.mode==='AM')am.classList.add('active');if(d.mode==='FM')fm.classList.add('active');if(d.mode==='LSB')lsb.classList.add('active');if(d.mode==='USB')usb.classList.add('active');
  rssiText.textContent=d.rssi;snrText.textContent=d.snr;chip.textContent=d.si4732?'Detected (0x11)':'Not detected';wifi.textContent=d.wifiMode;ip.textContent=d.ip;uptime.textContent=d.uptime;
  rssiBar.style.width=Math.max(0,Math.min(100,(d.rssi+120)*1.25))+'%';snrBar.style.width=Math.max(0,Math.min(100,(d.snr+20)*2.5))+'%';
  smode.textContent=d.mode;sfreq.textContent=d.freq+' kHz';sstep.textContent=d.step+' kHz';svol.textContent=d.muted?'MUTED':d.vol;sbfo.textContent=(d.bfo>=0?'+':'')+d.bfo+' Hz'+(d.bfoOn?' (ON)':' (OFF)');sbw.textContent=d.mode==='AM'?AM[d.bwAMIdx]+' kHz':(d.mode==='LSB'||d.mode==='USB'?SSB[d.bwSSBIdx]+' kHz':'--');ver.textContent='v'+d.version;
})}
setInterval(update,1000);update();loadMemInfo();
</script>
</body>
</html>
)rawliteral";

void handleRoot() {
  server.send_P(200, "text/html", webPage);
}

void handleStatus() {
  char json[420];
  unsigned long sec = millis() / 1000UL;
  unsigned long h = sec / 3600UL;
  unsigned long m = (sec % 3600UL) / 60UL;
  unsigned long s = sec % 60UL;
  sprintf(json,
    "{\"freq\":%u,\"mode\":\"%s\",\"vol\":%u,\"step\":%u,"
    "\"rssi\":%u,\"snr\":%u,\"muted\":%s,"
    "\"bfo\":%d,\"bfoOn\":%s,\"bwSSBIdx\":%u,\"bwAMIdx\":%u,"
    "\"version\":\"%s\",\"si4732\":%s,\"wifiMode\":\"%s\","
    "\"ip\":\"%s\",\"uptime\":\"%lu:%02lu:%02lu\"}",
    currentFrequency,
    modeStr[currentMode],
    volume,
    currentStep,
    rx.getCurrentRSSI(),
    rx.getCurrentSNR(),
    muted ? "true" : "false",
    currentBFO,
    bfoOn ? "true" : "false",
    bwIdxSSB,
    bwIdxAM,
    APP_VERSION,
    si4732Detected ? "true" : "false",
    webWiFiIsAP ? "AP" : "STA",
    webIPAddr.toString().c_str(),
    h, m, s);
  server.send(200, "application/json", json);
}

void handleCmd() {
  if (!server.hasArg("a")) {
    server.send(400, "text/plain", "missing a");
    return;
  }
  String action = server.arg("a");
  String val    = server.hasArg("v") ? server.arg("v") : "";

  lastActivity = millis();
  needSave = true;

  if (action == "tune") {
    int steps = val.toInt();
    int32_t nf = (int32_t)currentFrequency + steps * (int)currentStep;
    if (nf < band[bandIdx].minFreq) nf = band[bandIdx].minFreq;
    if (nf > band[bandIdx].maxFreq) nf = band[bandIdx].maxFreq;
    currentFrequency = (uint16_t)nf;
    rx.setFrequency(currentFrequency);
  }
  else if (action == "freq") {
    // Accept any frequency 150–30000 kHz (4 or 5 digits)
    long f = val.toInt();
    if (f >= 150 && f <= 30000) {
      currentFrequency = (uint16_t)f;
      // Auto-select band that contains this frequency (skip ALL first)
      int found = -1;
      for (int i = 0; i < lastBand; i++) {
        if (currentFrequency >= band[i].minFreq &&
            currentFrequency <= band[i].maxFreq) {
          found = i;
          break;
        }
      }
      if (found < 0) found = lastBand; // ALL band
      bandIdx = found;
      Band& b = band[bandIdx];
      if (currentMode == MODE_LSB || currentMode == MODE_USB) {
        loadSSB();
        rx.setSSB(b.minFreq, b.maxFreq, currentFrequency, currentStep,
                  currentMode == MODE_LSB ? LSB : USB);
        rx.setSSBAutomaticVolumeControl(1);
        rx.setSSBAudioBandwidth(bwIdxSSB);
      } else {
        rx.setAM(b.minFreq, b.maxFreq, currentFrequency, currentStep);
      }
      rx.setVolume(volume);
    }
  }
  else if (action == "mode") {
    if (val == "LSB") switchMode(MODE_LSB);
    else if (val == "USB") switchMode(MODE_USB);
    else if (val == "AM")  switchMode(MODE_AM);
    else if (val == "FM")  {
      // FM is retained as a UI option; use the 88–108 MHz range in 10 kHz units.
      currentMode = MODE_FM;
      currentFrequency = constrain(currentFrequency, (uint16_t)8800, (uint16_t)10800);
      rx.setFM(8800, 10800, currentFrequency, 10);
      rx.setVolume(volume);
      ssbLoaded = false;
      bfoOn = false;
    }
  }
  else if (action == "band") {
    int dir = val.toInt();
    bandIdx += dir;
    if (bandIdx < 0) bandIdx = lastBand;
    if (bandIdx > lastBand) bandIdx = 0;
    useBand();
  }
  else if (action == "vol") {
    int d = val.toInt();
    int nv = (int)volume + d;
    if (nv < 0) nv = 0;
    if (nv > 63) nv = 63;
    volume = nv;
    if (!muted) rx.setVolume(volume);
  }
  else if (action == "volset") {
    int nv = val.toInt();
    if (nv < 0) nv = 0;
    if (nv > 63) nv = 63;
    volume = nv;
    if (!muted) rx.setVolume(volume);
  }
  else if (action == "mute") {
    muted = !muted;
    if (muted) {
      rx.setVolume(0);
    } else {
      rx.setVolume(volume);
    }
  }
  else if (action == "step") {
    uint16_t s = val.toInt();
    if (s == 0) s = 1;
    currentStep = s;
    rx.setFrequencyStep(currentStep);
  }
  else if (action == "bfo") {
    if (currentMode != MODE_LSB && currentMode != MODE_USB) {
      server.send(400, "text/plain", "BFO is available in LSB/USB only");
      return;
    }
    if (val == "toggle") {
      bfoOn = !bfoOn;
      if (bfoOn) rx.setSSBBfo(currentBFO);
    } else {
      int delta = val.toInt();
      currentBFO += delta;
      if (currentBFO > 16000) currentBFO = 16000;
      if (currentBFO < -16000) currentBFO = -16000;
      rx.setSSBBfo(currentBFO);
      bfoOn = true;
    }
  }
  else if (action == "bfoSet") {
    if (currentMode != MODE_LSB && currentMode != MODE_USB) {
      server.send(400, "text/plain", "BFO is available in LSB/USB only");
      return;
    }
    currentBFO = val.toInt();
    if (currentBFO > 16000) currentBFO = 16000;
    if (currentBFO < -16000) currentBFO = -16000;
    rx.setSSBBfo(currentBFO);
    bfoOn = true;
    needSave = true;
    server.send(200, "text/plain", "BFO set");
    return;
  }
  else if (action == "bwssb") {
    uint8_t idx = constrain(val.toInt(), 0, 5);
    bwIdxSSB = idx;
    if (currentMode == MODE_LSB || currentMode == MODE_USB) {
      rx.setSSBAudioBandwidth(bwIdxSSB);
      rx.setSSBSidebandCutoffFilter((bwIdxSSB == 0 || bwIdxSSB == 4 || bwIdxSSB == 5) ? 0 : 1);
    }
  }
  else if (action == "bwam") {
    uint8_t idx = constrain(val.toInt(), 0, 6);
    bwIdxAM = idx;
    if (currentMode == MODE_AM) {
      rx.setBandwidth(bwIdxAM, 1);
    }
  }
  else if (action == "mem_store") {
    uint8_t slot = val.toInt();
    if (slot < NUM_MEM) {
      saveMemorySlot(slot);
      char msg[48];
      sprintf(msg, "Stored %u %s -> M%u", currentFrequency, modeStr[currentMode], slot + 1);
      showStatus();
      server.send(200, "text/plain", msg);
      return;
    }
  }
  else if (action == "mem_recall") {
    uint8_t slot = val.toInt();
    if (slot < NUM_MEM) {
      if (memFreq[slot] == 0) {
        server.send(200, "text/plain", "M slot empty");
        return;
      }
      recallMemory(slot);
      char msg[48];
      sprintf(msg, "Recalled M%u: %u %s", slot + 1, currentFrequency, modeStr[currentMode]);
      server.send(200, "text/plain", msg);
      return;
    }
  }

  showStatus();
  server.send(200, "text/plain", "OK");
}

void handleMem() {
  // JSON array of memory slots
  String json = "[";
  for (uint8_t i = 0; i < NUM_MEM; i++) {
    if (i) json += ",";
    json += "{\"freq\":";
    json += memFreq[i];
    json += ",\"mode\":\"";
    json += (memFreq[i] ? modeStr[memMode[i]] : "-");
    json += "\"}";
  }
  json += "]";
  server.send(200, "application/json", json);
}
