// TFT-Driver-Mk1
// https://github.com/jwidess/ESP-Arduino-TFT-Weather
// Author: Jwidess
// This sketch is for a 2.4" 320x240 TFT ILI9341 display on an Arduino Uno.
// ==========================================================================

#include <Arduino.h>

#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;
#include "bitmaps.h"
#include <SPI.h>

// Assign human-readable names to some common 16-bit color values:
#define BLACK     0x0000
#define GREY      0x73AE
#define DARKGREY  0x2104
#define BLUE      0x001F
#define LIGHTBLUE 0x055E
#define RED       0xf800
#define LIGHTRED  0xfa09
#define GREEN     0x07E0
#define CYAN      0x07FF
#define MAGENTA   0xF81F
#define YELLOW    0xFFE0
#define WHITE     0xFFFF

// 2.4" 320x240 TFT display configuration
#define TFT_WIDTH  240 // Width in pixels
#define TFT_HEIGHT 320 // Height in pixels

/*
#include <TouchScreen.h>
#define MINPRESSURE 200
#define MAXPRESSURE 1000
// Touchscreen configuration
char *name = "2.4 TFT ILI9341";

// From calibration on 6/28/2025
const int   XP = 8, XM = A2, YP = A3, YM = 9; // 240x320 ID=0x9341
const int   TS_LEFT = 155, TS_RT = 856, TS_TOP = 90, TS_BOT = 845;
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

int  pixel_x, pixel_y; // Touch_getXY() updates global vars
bool Touch_getXY(void) {
  TSPoint p = ts.getPoint();
  pinMode(YP, OUTPUT); // restore shared pins
  pinMode(XM, OUTPUT);
  digitalWrite(YP, HIGH); // because TFT control pins
  digitalWrite(XM, HIGH);
  bool pressed = (p.z > MINPRESSURE && p.z < MAXPRESSURE);
  if (pressed) {
    pixel_x = map(p.x, TS_LEFT, TS_RT, 0, tft.width()); //.kbv makes sense to me
    pixel_y = map(p.y, TS_TOP, TS_BOT, 0, tft.height());
  }
  return pressed;
}
*/

// Debug print enable
//! Enabling this adds 11.2% of RAM and 1.5% of flash usage
#define DEBUG_PRINT 0 // Set to 1 to enable debug prints

// Function prototypes
void drawTextCentered(const char *text, int y, int textSize, uint16_t color);
void drawWeatherUI();
void parseWeatherData(const String &input);
void drawStatusIndicator();
void drawTemperatureWithDegreeF(const char *tempStr, int x, int y, int textSize, uint16_t color);
void drawWeatherIconForCloudText();

// Lopaka UI text variables (https://lopaka.app/gallery/10323/21464)
char TempHIGH_text[5] = "HI";
char TempLOW_text[5] = "LO";
char CurrentTEMP_text[5] = "TEMP";
char Chance_of_Rain_Number_text[6] = "RAIN";
char Humidity_Number_text[6] = "HUMID";
char Time_text[12] = "TIME";
char Week_Month_Day_text[32] = "WEEK";
char Date_text[14] = "DATE";
char Cloud_text[20] = "CLOUDY";

// Track last UART RX time. If no UART message is received after 2 minutes, set Time_text to ERROR and raise flag
unsigned long lastUartRx = 0;
bool uartTimeoutFlag = false;

bool minorErrorFlag = false; // Flag for minor errors, e.g. parsing issues

// Status indicator animation state
uint8_t statusAnimIdx = 0;

// ==========================================================================
// Setup & Loop:

void setup(void) {
  Serial.begin(9600);
  uint16_t ID = tft.readID();
  Serial.println("=============================");
  Serial.println("Booting TFT-Driver-Mk1...");
  Serial.print("TFT ID = 0x");
  Serial.println(ID, HEX);
  if (DEBUG_PRINT) Serial.println("!! DEBUG MODE ENABLED !!");
  Serial.println("=============================");
  if (ID == 0xD3D3) ID = 0x9486; // write-only shield
  tft.begin(ID);
  tft.setRotation(0); // PORTRAIT
  tft.fillScreen(BLACK);
  drawWeatherUI();
}

void loop(void) {
  static String input = "";
  static unsigned long lastStatusAnim = 0;
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      parseWeatherData(input);
      drawWeatherUI();
      input = "";
    } else {
      input += c;
    }
  }
  // Check for UART timeout (no message in over 2 minutes)
  if (millis() - lastUartRx > 120000UL) { // 120000ms = 2 minutes
    if (!uartTimeoutFlag) {
      strncpy(Time_text, "ERROR", sizeof(Time_text));
      uartTimeoutFlag = true;
      drawWeatherUI();
    }
  }
  // Animate status indicator every 100ms
  if (millis() - lastStatusAnim > 100) {
    drawStatusIndicator();
    lastStatusAnim = millis();
  }
}

// ==========================================================================
// Functions:

void drawWeatherUI() {
  tft.fillScreen(BLACK); // Clear screen (Not optimal)

  // Outline for centering in enclosure
  if (DEBUG_PRINT) tft.drawRect(0, 0, TFT_WIDTH, TFT_HEIGHT, 0xF206);

  // Cloud Cover Symbol (dynamic)
  drawWeatherIconForCloudText();

  // Vert Divider
  // tft.drawFastVLine(113, 8, 88, WHITE); // Spaced line
  tft.drawFastVLine(115, 1, 103, WHITE); // Fully connected line

  int Deg_F_Offset = 16; // Offset for degree and F when printing in drawTemperatureWithDegreeF
  // TempHIGH (right aligned, 5px padding)
  int tempHIGHLen = strlen(TempHIGH_text);
  int tempHIGHWidth = tempHIGHLen * 6 * 5;
  drawTemperatureWithDegreeF(TempHIGH_text, TFT_WIDTH - tempHIGHWidth - Deg_F_Offset, 10, 5, LIGHTRED);

  // TempLOW (right aligned, 5px padding)
  int tempLowLen = strlen(TempLOW_text);
  int tempLowWidth = tempLowLen * 6 * 5;
  drawTemperatureWithDegreeF(TempLOW_text, TFT_WIDTH - tempLowWidth - Deg_F_Offset, 57, 5, LIGHTBLUE);

  // Above Temp Divider
  // tft.drawLine(5, 104, 234, 104, WHITE);
  tft.drawFastHLine(5, 104, 229, WHITE);

  // Current TEMP
  int currentTempLen = strlen(CurrentTEMP_text);
  int currentTempWidth = currentTempLen * 6 * 7;
  drawTemperatureWithDegreeF(CurrentTEMP_text, (TFT_WIDTH - currentTempWidth) / 2, 115, 7, WHITE);

  // Below Temp Divider
  // tft.drawLine(5, 173, 234, 173, WHITE);
  tft.drawFastHLine(5, 173, 229, WHITE);

  // Humidity Symbol
  tft.drawBitmap(5, 180, image_Humidity_Symbol_bits, 17, 24, WHITE);

  // Humidity Number
  tft.setTextColor(WHITE);
  tft.setTextSize(3);
  tft.setCursor(30, 182);
  tft.print(Humidity_Number_text);

  // Chance of Rain Symbol and Number (right-aligned together)
  tft.setTextSize(3);
  int rainNumLen = strlen(Chance_of_Rain_Number_text);
  int rainNumWidth = rainNumLen * 6 * 3;
  int rainNumRight = 235; // right edge for both symbol and text
  int rainNumX = rainNumRight - rainNumWidth;
  int rainIconWidth = 30;                       // width of the bitmap
  int rainIconX = rainNumX - rainIconWidth - 7; // 7px spacing

  // Determine color based on rain chance
  uint16_t rainColor = WHITE;
  int rainPercent = 0;
  // Extract number value from Chance_of_Rain_Number_text (e.g, "55%")
  // Yes this is a bit hacky, but it works
  char percentStr[4] = {0};
  for (int i = 0, j = 0; Chance_of_Rain_Number_text[i] != '\0' && j < 3; ++i) {
    if (isdigit(Chance_of_Rain_Number_text[i])) {
      percentStr[j++] = Chance_of_Rain_Number_text[i];
    } else {
      break;
    }
  }
  if (percentStr[0] != '\0') {
    rainPercent = atoi(percentStr);
    if (rainPercent >= 50) { rainColor = LIGHTRED; }
  }

  tft.drawBitmap(rainIconX, 177, image_Chance_of_Rain_Symbol_bits, rainIconWidth, 32, rainColor);
  tft.setCursor(rainNumX, 183);
  tft.setTextColor(rainColor);
  tft.print(Chance_of_Rain_Number_text);

  // Below Humid Divider
  // tft.drawLine(5, 212, 234, 212, WHITE);
  tft.drawFastHLine(5, 212, 229, WHITE);

  // Time (centered)
  drawTextCentered(Time_text, 227, 4, uartTimeoutFlag ? RED : WHITE);

  // Week_Month_Day (centered) E.g. "Monday - Jan 17th"
  drawTextCentered(Week_Month_Day_text, 275, 2, WHITE);

  // Date Underline
  // tft.drawLine(20, 294, 219, 294, WHITE);
  tft.drawFastHLine(20, 294, 200, WHITE); // Centered underline

  // Date (centered) E.g. "1/17/2025"
  drawTextCentered(Date_text, 301, 2, WHITE);

  // Draw minor error indicator if needed
  if (minorErrorFlag) {
    tft.fillCircle(232, 312, 4, YELLOW); // Small yellow circle, bottom right
  }
  minorErrorFlag = false; // Reset after drawing
}

// Draw text centered horizontally on the display
// @param text      The string to print
// @param y         The y position (top of text)
// @param textSize  The text size (as used by setTextSize)
// @param color     The 16-bit color value
void drawTextCentered(const char *text, int y, int textSize, uint16_t color) {
  tft.setTextSize(textSize);
  int len = strlen(text);
  int textWidth = len * 6 * textSize;
  int x = (TFT_WIDTH - textWidth) / 2;
  tft.setTextColor(color);
  tft.setCursor(x, y);
  tft.print(text);
}

// Parse UART weather data and update display variables
void parseWeatherData(const String &input) {
  lastUartRx = millis();

  size_t idx = 0;
  String data = input;
  data.trim();
  while (idx < data.length()) {
    int sep = data.indexOf(',', idx);
    String pair = (sep == -1) ? data.substring(idx) : data.substring(idx, sep);
    int colon = pair.indexOf(':');
    if (colon > 0) {
      String key = pair.substring(0, colon);
      String value = pair.substring(colon + 1);
      value.trim();
      if (value.length() == 0) {
        minorErrorFlag = true; // Set error flag if any value is empty
      }
      if (key == "HIGH") {
        value.toCharArray(TempHIGH_text, sizeof(TempHIGH_text));
      } else if (key == "LOW") {
        value.toCharArray(TempLOW_text, sizeof(TempLOW_text));
      } else if (key == "TEMP") {
        value.toCharArray(CurrentTEMP_text, sizeof(CurrentTEMP_text));
      } else if (key == "RAIN") {
        snprintf(Chance_of_Rain_Number_text, sizeof(Chance_of_Rain_Number_text), "%s%%", value.c_str());
      } else if (key == "HUM") {
        snprintf(Humidity_Number_text, sizeof(Humidity_Number_text), "%s%%", value.c_str());
      } else if (key == "TIME") {
        uartTimeoutFlag = false;
        value.toCharArray(Time_text, sizeof(Time_text));
      } else if (key == "DATE") {
        value.toCharArray(Week_Month_Day_text, sizeof(Week_Month_Day_text));
      } else if (key == "DATETXT") {
        value.toCharArray(Date_text, sizeof(Date_text));
      } else if (key == "CLOUD") {
        value.toCharArray(Cloud_text, sizeof(Cloud_text));
      }
    }
    if (sep == -1) break;
    idx = sep + 1;
  }

  if (DEBUG_PRINT) {
    Serial.println("--- UART RX Payload ---");
    Serial.println(input);
    Serial.println("-----------------------");
    // Print formatted values for debugging
    // clang-format off
    Serial.println("--- Weather Data ---");
    Serial.print("TempHIGH_text: "); Serial.println(TempHIGH_text);
    Serial.print("TempLOW_text: "); Serial.println(TempLOW_text);
    Serial.print("CurrentTEMP_text: "); Serial.println(CurrentTEMP_text);
    Serial.print("Chance_of_Rain_Number_text: "); Serial.println(Chance_of_Rain_Number_text);
    Serial.print("Humidity_Number_text: "); Serial.println(Humidity_Number_text);
    Serial.print("Time_text: "); Serial.println(Time_text);
    Serial.print("Week_Month_Day_text: "); Serial.println(Week_Month_Day_text);
    Serial.print("Date_text: "); Serial.println(Date_text);
    Serial.print("Cloud_text: "); Serial.println(Cloud_text);
    Serial.println("--------------------");
    // clang-format on
  }
}

void drawStatusIndicator() {
  // Classic spinner: "|", "/", "-", "\\":
  static const char spinnerChars[4] = {'|', '/', '-', '\\'};
  tft.setTextSize(1);
  tft.setTextColor(WHITE, BLACK); // Draw white on black to erase previous
  tft.setCursor(3, 310);          // Bottom left corner
  tft.print(spinnerChars[statusAnimIdx % 4]);
  statusAnimIdx = (statusAnimIdx + 1) % 4;
}

void drawTemperatureWithDegreeF(const char *tempStr, int x, int y, int textSize, uint16_t color) {
  if (textSize < 3 || textSize > 7) {
    Serial.println("Error: textSize must be between 3 and 7");
    tft.setTextSize(textSize);
    tft.setTextColor(color, BLACK);
    tft.setCursor(x, y);
    tft.print("ERR");
    return;
  }

  int tempLen = strlen(tempStr);
  int charWidth = 6 * textSize; // Each char is 6px wide in Adafruit_GFX
  int totalWidth = tempLen * charWidth;
  int totalHeight = textSize * 7; // Height in pixels based on text size

  // Draw the temperature string
  tft.setTextSize(textSize);
  tft.setTextColor(color, BLACK);
  tft.setCursor(x, y);
  tft.print(tempStr);

  // Calculate where to draw the degree symbol
  int degRadius = 3;  // Default
  uint8_t F_size = 2; // Default size for 'F'
  // clang-format off
  switch (textSize) {
    case 3: degRadius = 3; F_size = 1; break;
    case 4: degRadius = 4; F_size = 1; break;
    case 5: degRadius = 5; F_size = 2; break;
    case 6: degRadius = 5; F_size = 2; break;
    case 7: degRadius = 5; F_size = 2; break;
  }
  // clang-format on

  int degX = x + totalWidth + degRadius; // Small gap after temp
  int degY = y + degRadius;              // Vertically aligned with text

  tft.drawCircle(degX, degY, degRadius, WHITE);

  int F_X = x + totalWidth;                   // Small gap after temp
  int F_Y = (y + totalHeight) - (F_size * 7); // Bottom-aligned with temp text

  tft.setTextSize(F_size);
  tft.setTextColor(DARKGREY, BLACK);

  tft.setCursor(F_X, F_Y);
  tft.print('F');
}

// Struct for weather icon mapping
struct WeatherIconMapping {
  const char *keywords[6]; // Up to 5 keywords, null-terminated
  const uint8_t *bitmap;
  int x, y, w, h;
};

// Icon mappings table
const WeatherIconMapping iconMappings[] = {
    {{"rain", "rainy", "pouring", "hail", nullptr}, image_weather_raining_bits, 14, 10, 85, 80},
    {{"lightning", "lightning-rainy", "thunder", nullptr}, image_weather_lightning_bits, 12, 10, 90, 85},
    {{"snowy", "snowy-rainy", nullptr}, image_weather_snow_bits, 12, 6, 90, 96},
    {{"partlycloudy", "cloudy-sunny", nullptr}, image_weather_cloudy_sunny_bits, 12, 10, 85, 80},
    {{"cloudy", "fog", "windy", "windy-variant", nullptr}, image_weather_cloud_bits, 12, 20, 85, 80},
    {{"exceptional", "unknown", nullptr}, image_weather_exceptional_bits, 12, 6, 90, 90},
    {{"clear-night", "moon", "night", nullptr}, image_weather_moon_bits, 16, 15, 80, 80},
    {{"sunny", "clear", nullptr}, image_weather_sunny_bits, 12, 5, 90, 96},
    {{nullptr}, nullptr, 0, 0, 0, 0} // Sentinel (all null)
};

// Helper function to draw the correct weather icon based on Cloud_text
void drawWeatherIconForCloudText() {
  String cloud = String(Cloud_text);
  cloud.toLowerCase();
  for (int i = 0; iconMappings[i].keywords[0] != nullptr; ++i) {
    for (int k = 0; iconMappings[i].keywords[k] != nullptr; ++k) {
      if (cloud.indexOf(iconMappings[i].keywords[k]) != -1) {
        tft.drawBitmap(iconMappings[i].x, iconMappings[i].y, iconMappings[i].bitmap, iconMappings[i].w, iconMappings[i].h, 0xFFFF);
        return;
      }
    }
  }
  // Default fallback
  tft.drawBitmap(12, 20, image_weather_cloud_bits, 85, 80, 0xFFFF);
  minorErrorFlag = true;
  Serial.println("Warning: Unrecognized cloud text: " + cloud);
}
