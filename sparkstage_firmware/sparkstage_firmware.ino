/*

書き込みの設定
 - USB DFU On Boot : Enabled
 - USB Mode : USB-OTG

*/

// Grove利用例：M5StackのDUAL BUTTON UNITをM5StampS3すぐ横のGroveコネクタに接続してください
// #define USE_DUAL_BUTTON_UNIT

#include <Preferences.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include "www_data.h"

#define WIFI_AP_SSID "SparkStage"
#define WIFI_AP_PASSWORD "12345678"

IPAddress routerIPAddress(192, 168, 4, 1);
IPAddress gatewayIPAddress(0, 0, 0, 0);
IPAddress subnetMask(255, 255, 255, 0);

WebServer httpServer(80);

#define DEBUG_SERIAL Serial

#include <Adafruit_NeoPixel.h>

#define PIN_NEOPIXEL_BUILTIN 21
#define PIN_NEOPIXEL_MATRIX 1
#define NEOPIXEL_MATRIX_COUNT 25

Adafruit_NeoPixel led(1, PIN_NEOPIXEL_BUILTIN, NEO_GRB | NEO_KHZ800);
Adafruit_NeoPixel matrix(NEOPIXEL_MATRIX_COUNT, PIN_NEOPIXEL_MATRIX, NEO_GRB | NEO_KHZ800);

typedef enum MATRIX_MODE_enum {
  MODE_RAINBOW = 0,
  MODE_SINGLECOLOR,
  MODE_BLINK,
  MODE_WAVE,
  MODE_END
} MATRIX_MODE_t;

MATRIX_MODE_t matrix_mode = MODE_RAINBOW;

typedef struct color_st {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
} color_t;

color_t current_color;

Preferences preferences;

String configString;

void led_clear(void) {
  led.begin();
  led.clear();
  led.show();

  matrix.begin();
  matrix.setBrightness(128);
  matrix.clear();
  matrix.show();
}

void update_matrix_operation() {
  // "MODE:1,COLOR:0123ff"
  if (configString.equals("")) {
    return;
  }
  String modeString = configString.substring(5, 6);
  int mode = modeString.toInt();
  if (MODE_RAINBOW <= mode && mode < MODE_END) {
    matrix_mode = (MATRIX_MODE_t)mode;
  } else {
    matrix_mode = MODE_RAINBOW;
  }
  String colorString = configString.substring(13, 19);
  long color = strtol(colorString.c_str(), NULL, 16);
  current_color.red = (color >> 16) & 0xff;
  current_color.green = (color >> 8) & 0xff;
  current_color.blue = color & 0xff;

  DEBUG_SERIAL.printf("update_matrix_operation : MODE=%d, COLOR=%02x%02x%02x\n", matrix_mode, current_color.red, current_color.green, current_color.blue);
}


String getWiFiSTASSID() {
  if (preferences.isKey("wifistassid")) {
    return preferences.getString("wifistassid");
  } else {
    return String("");
  }
}

String getWiFiSTAPSK() {
  if (preferences.isKey("wifistapsk")) {
    return preferences.getString("wifistapsk");
  } else {
    return String("");
  }
}

void setWiFiSTASSID(String ssid) {
  preferences.putString("wifistassid", ssid);
}

void setWiFiSTAPSK(String psk) {
  preferences.putString("wifistapsk", psk);
}


#include "esp_mac.h"

String getUniqueIDString() {
  uint8_t macaddr[6] = {};
  if (esp_efuse_mac_get_default(macaddr) == ESP_OK) {
    char buf[7] = {};
    snprintf(buf, 7, "%02x%02x%02x", macaddr[3], macaddr[4], macaddr[5]);

    return String(buf);
  } else {
    return String("051");
  }
}


String getWiFiAPSSID() {
  String ssid = "SparkStage-";
  ssid += getUniqueIDString();
  return ssid;
}

String getMDNSHostName() {
  String hostname = "sparkstage-";
  hostname += getUniqueIDString();
  return hostname;
}


void handleRoot() {
  httpServer.send(200, "text/html", index_html);
}

void handleStylesheetFile() {
  httpServer.send(200, "text/css", style_css);
}

void handleScriptFile() {
  httpServer.send(200, "text/javascript", script_js);
}

void handleSetWiFiConfig() {
  DEBUG_SERIAL.println("handleSetWiFiConfig()");
  
  if (!httpServer.hasArg("s")) {
    httpServer.send(400, "text/plain", "parameter is missing.");
    return;
  }
  if (!httpServer.hasArg("p")) {
    httpServer.send(400, "text/plain", "parameter is missing.");
    return;
  }
  String configSSID = httpServer.arg("s");
  String configPSK = httpServer.arg("p");

  setWiFiSTASSID(configSSID);
  setWiFiSTAPSK(configPSK);

  httpServer.send(200, "text/plain", "OK.");
}

void handleSetConfig() {
  DEBUG_SERIAL.println("Request Set config");
  if (!httpServer.hasArg("config")) {
    httpServer.send(400, "text/plain", "config parameter is missing.");
    return;
  }
  configString = httpServer.arg("config");  
  DEBUG_SERIAL.printf("config=\"%s\"\n", configString.c_str());
  update_matrix_operation();
  httpServer.send(200, "text/plain", "OK.");
}

void handleGetConfig() {
  DEBUG_SERIAL.println("Request Get config");
  httpServer.send(200, "text/plain", configString);
}

void handleSaveConfig() {
  DEBUG_SERIAL.println("Request Save config");
  if (!httpServer.hasArg("config")) {
    httpServer.send(400, "text/plain", "config parameter is missing.");
    return;
  }
  configString = httpServer.arg("config");
  DEBUG_SERIAL.printf("config=\"%s\"\n", configString.c_str());
  preferences.putString("config", configString);
  update_matrix_operation();
  httpServer.send(200, "text/plain", "OK.");
}

void handleNotFound() {
  httpServer.send(404, "text/plain", "404 Not found.");
}

#ifdef USE_DUAL_BUTTON_UNIT
void handle_dual_button_unit(void* param) {
  while (true) {
    if (digitalRead(15) == LOW) {
      uint32_t colors[] = { 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00ffff00, 0x00ff00ff, 0x0000ffff, 0x00ffffff };
      const size_t color_count = sizeof(colors) / sizeof(colors[0]);
      static int color_index = 0;
      uint32_t color = colors[color_index];
      current_color.red = (color >> 16);
      current_color.green = (color >> 8);
      current_color.blue = (color & 0xff);
      color_index = (color_index + 1) % color_count;
      while (digitalRead(15) == LOW) { delay(5); }

    } else if (digitalRead(13) == LOW) {
      static int mode_index = 0;
      int mode_count = 3;  // MODE_RAINBOW, MODE_SINGLECOLOR, MODE_BLINK
      switch (mode_index) {
      case 0:
        matrix_mode = MODE_SINGLECOLOR;
        break;
      case 1:
        matrix_mode = MODE_BLINK;
        break;
      case 2:
      default:
        matrix_mode = MODE_RAINBOW;
        break;
      }
      mode_index = (mode_index + 1) % mode_count;
      while (digitalRead(13) == LOW) { delay(5); }
    }

    delay(10);
  }
}
#endif

void setup() {
  DEBUG_SERIAL.begin(115200);
  led_clear();

  for (int i = 0; i < 4; i++) {
    led.setPixelColor(0, i % 2 == 0 ? led.Color(32, 32, 32) : led.Color(0, 0, 0));
    led.show();
    delay(200);
  }

  DEBUG_SERIAL.println("Initializing...");


#ifdef USE_DUAL_BUTTON_UNIT
  pinMode(15, INPUT_PULLUP);
  pinMode(13, INPUT_PULLUP);
#endif

  {
    preferences.begin("sparkstage", false);
    if (!preferences.isKey("config")) {
      preferences.putString("config", "MODE:0,COLOR:7f7f7f");
    }
    configString = preferences.getString("config");
    update_matrix_operation();
  }


  xTaskCreatePinnedToCore(neopixel_task, "neopixel_task", 4096, NULL, 1, NULL, 1);
#ifdef USE_DUAL_BUTTON_UNIT
  xTaskCreatePinnedToCore(handle_dual_button_unit, "dualbutton_task", 4096, NULL, 2, NULL, 1);
#endif

  bool startWiFiAP = true;

  {
    String staSSID = getWiFiSTASSID();
    if (staSSID.length() > 0) {
      String staPSK = getWiFiSTAPSK();
      DEBUG_SERIAL.printf("Try to connect WiFi %s\r\n", staSSID.c_str());

      led.setPixelColor(0, led.Color(32, 0, 0));
      led.show();
      WiFi.mode(WIFI_STA);
      WiFi.begin(staSSID, staPSK);
      for (unsigned long start = millis(); (WiFi.status() != WL_CONNECTED) &&  (millis() - start < 10 * 1000); ) {
        led.setPixelColor(0, led.Color(0, 0, 0));
        led.show();
        delay(250);
        led.setPixelColor(0, led.Color(128, 0, 0));
        led.show();
      }
      led.setPixelColor(0, led.Color(32, 0, 0));
      led.show();
      if (WiFi.status() == WL_CONNECTED) {
        startWiFiAP = false;
      }
    }

  }

  if (startWiFiAP) {
    // DEBUG_SERIAL.println("Start WiFi AP : " WIFI_AP_SSID " / " WIFI_AP_PASSWORD);
    DEBUG_SERIAL.printf("Start WiFI AP: %s / %s\r\n", getWiFiAPSSID().c_str(), WIFI_AP_PASSWORD);

    WiFi.mode(WIFI_AP);
    WiFi.softAP(getWiFiAPSSID().c_str(), WIFI_AP_PASSWORD);
    
    led.setPixelColor(0, led.Color(0, 32, 0));
    led.show();
    delay(100);
    led.setPixelColor(0, led.Color(0, 0, 0));
    led.show();
    WiFi.softAPConfig(routerIPAddress, gatewayIPAddress, subnetMask);
  }


  {
    DEBUG_SERIAL.println("Start HTTP Server");

    httpServer.on("/", HTTP_GET, handleRoot);
    httpServer.on("/style.css", HTTP_GET, handleStylesheetFile);
    httpServer.on("/script.js", HTTP_GET, handleScriptFile);
    httpServer.on("/config", HTTP_POST, handleSetConfig);
    httpServer.on("/config", HTTP_GET, handleGetConfig);
    httpServer.on("/config_save", HTTP_POST, handleSaveConfig);
    httpServer.on("/config_wifi", HTTP_POST, handleSetWiFiConfig);

    httpServer.begin();
  }


  {
    MDNS.begin(getMDNSHostName().c_str());
    MDNS.addService("https", "tcp", 80);

    DEBUG_SERIAL.printf("mDNS advertise: %s.local\r\n", getMDNSHostName().c_str());
  }

  
  led.setPixelColor(0, led.Color(32, 32, 32));
  led.show();
  delay(200);
  led.setPixelColor(0, led.Color(0, 0, 0));
  led.show();

  DEBUG_SERIAL.println("Ready.");
}



void loop() {
  httpServer.handleClient();
}



void neopixel_task(void* arg) {
  while (true) {
    while (matrix_mode == MODE_RAINBOW) {
      // Some example procedures showing how to display to the pixels:
      colorWipe(matrix.Color(255, 0, 0), 50);// Red
      if (matrix_mode != MODE_RAINBOW) { break; }
      colorWipe(matrix.Color(0, 255, 0), 50); // Green
      if (matrix_mode != MODE_RAINBOW) { break; }
      colorWipe(matrix.Color(0, 0, 255), 50); // Blue
      if (matrix_mode != MODE_RAINBOW) { break; }

      // Send a theater pixel chase in...
      theaterChase(matrix.Color(127, 127, 127), 70); // White
      if (matrix_mode != MODE_RAINBOW) { break; }
      theaterChase(matrix.Color(255, 0, 0), 70); // Red
      if (matrix_mode != MODE_RAINBOW) { break; }
      theaterChase(matrix.Color(0, 255, 0), 70); // GREEN
      if (matrix_mode != MODE_RAINBOW) { break; }
      theaterChase(matrix.Color(0, 0, 255), 70); // Blue
      if (matrix_mode != MODE_RAINBOW) { break; }

      rainbow(10);
      if (matrix_mode != MODE_RAINBOW) { break; }
      rainbowCycle(10);
      if (matrix_mode != MODE_RAINBOW) { break; }
      theaterChaseRainbow(50);
      if (matrix_mode != MODE_RAINBOW) { break; }
    }

    while (matrix_mode == MODE_SINGLECOLOR) {
      for (int i = 0; i < 25; i++) {
        matrix.setPixelColor(i, matrix.Color(current_color.red, current_color.green, current_color.blue));
      }
      matrix.show();
      delay(10);
    }

    while (matrix_mode == MODE_BLINK) {
      for (uint8_t m = 0; matrix_mode == MODE_BLINK; m++) {
        for (int i = 0; i < 25; i++) {
          if ((m & 0x01) == 0) {
            matrix.setPixelColor(i, matrix.Color(current_color.red, current_color.green, current_color.blue));
          } else {
            matrix.setPixelColor(i, matrix.Color(0, 0, 0));
          }
        }
        matrix.show();
        for (int j = 0; j < 50; j++) {
          delay(10);
          if (matrix_mode != MODE_BLINK) {
            break;
          }
        }
      }
    }
  }
}


// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<matrix.numPixels(); i++) {
    matrix.setPixelColor(i, c);
    matrix.show();
    delay(wait);
    if (matrix_mode != MODE_RAINBOW) { return; }
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<matrix.numPixels(); i++) {
      matrix.setPixelColor(i, Wheel((i+j) & 255));
    }
    matrix.show();
    delay(wait);
    if (matrix_mode != MODE_RAINBOW) { return; }
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< matrix.numPixels(); i++) {
      matrix.setPixelColor(i, Wheel(((i * 256 / matrix.numPixels()) + j) & 255));
    }
    matrix.show();
    delay(wait);
    if (matrix_mode != MODE_RAINBOW) { return; }
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < matrix.numPixels(); i=i+3) {
        matrix.setPixelColor(i+q, c);    //turn every third pixel on
      }
      matrix.show();

      delay(wait);
      if (matrix_mode != MODE_RAINBOW) { return; }

      for (uint16_t i=0; i < matrix.numPixels(); i=i+3) {
        matrix.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < matrix.numPixels(); i=i+3) {
        matrix.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      matrix.show();

      delay(wait);
      if (matrix_mode != MODE_RAINBOW) { return; }

      for (uint16_t i=0; i < matrix.numPixels(); i=i+3) {
        matrix.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return matrix.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return matrix.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return matrix.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
