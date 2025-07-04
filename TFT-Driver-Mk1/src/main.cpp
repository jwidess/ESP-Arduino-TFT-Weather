// TFT-Driver-Mk1
// https://github.com/jwidess/ESP-Arduino-TFT-Weather
// Author: Jwidess
// This sketch is for a 2.4" 320x240 TFT ILI9341 display with touch panel on an Arduino Uno.
// ==========================================================================

#include <Arduino.h>

#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;
#include "bitmaps.h"
#include <SPI.h>
// #include <TouchScreen.h>

#define MINPRESSURE 200
#define MAXPRESSURE 1000

// Assign human-readable names to some common 16-bit color values:
#define BLACK   0x0000
#define GREY    0x73AE
#define DARKGREY 0x2104
#define BLUE    0x001F
#define LIGHTBLUE 0x055E
#define RED     0xf800
#define LIGHTRED 0xfa09
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

/*
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

// Function prototypes
void drawTextCentered(const char *text, int y, int textSize, uint16_t color);
void drawTextAligned(const char *text, int y, int textSize, uint16_t color, const char *align);
void drawWeatherUI();
void parseWeatherData(const String &input);
void drawStatusIndicator();
void drawTemperatureWithDegreeF(const char *tempStr, int x, int y, int textSize, uint16_t color);
void drawWeatherIconForCloudText();

// Lopaka UI text variables
char TempHIGH_text[8]              = "HI";
char TempLOW_text[8]               = "LO";
char CurrentTEMP_text[8]           = "TEMP";
char Chance_of_Rain_Number_text[8] = "RAIN";
char Humidity_Number_text[8]       = "HUMID";
char Time_text[16]                 = "TIME";
char Week_Month_Day_text[32]       = "WEEK";
char Date_text[16]                 = "DATE";
char Cloud_text[20]                = "CLOUDY";

// Track last UART RX time. If no UART message is received after 2 minutes, set Time_text to ERROR and raise flag
unsigned long lastUartRx      = 0;
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
  Serial.println("TFT ID = 0x");
  Serial.println("=============================");
  Serial.println(ID, HEX);
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
  tft.fillScreen(BLACK);
  // Outline
  tft.drawRect(0, 0, 240, 320, 0xF206);

  // Cloud Cover Symbol (dynamic)
  drawWeatherIconForCloudText();

  // Vert Divider
  //tft.drawFastVLine(113, 8, 88, WHITE); // Spaced line
  tft.drawFastVLine(115, 1, 103, WHITE); // Fully connected line

  int Deg_F_Offset = 16; // Offset for degree and F when printing in drawTemperatureWithDegreeF
  // TempHIGH (right aligned, 5px padding)
  int tempHIGHLen   = strlen(TempHIGH_text);
  int tempHIGHWidth = tempHIGHLen * 6 * 5;
  drawTemperatureWithDegreeF(TempHIGH_text, 240 - tempHIGHWidth - Deg_F_Offset, 10, 5, LIGHTRED);

  // TempLOW (right aligned, 5px padding)
  int tempLowLen   = strlen(TempLOW_text);
  int tempLowWidth = tempLowLen * 6 * 5;
  drawTemperatureWithDegreeF(TempLOW_text, 240 - tempLowWidth - Deg_F_Offset, 57, 5, LIGHTBLUE);

  // Above Temp Divider
  // tft.drawLine(5, 104, 234, 104, WHITE);
  tft.drawFastHLine(5, 104, 229, WHITE);

  // Current TEMP
  int currentTempLen   = strlen(CurrentTEMP_text);
  int currentTempWidth = currentTempLen * 6 * 7;
  drawTemperatureWithDegreeF(CurrentTEMP_text, (240 - currentTempWidth) / 2, 115, 7, WHITE);

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
  int rainNumLen    = strlen(Chance_of_Rain_Number_text);
  int rainNumWidth  = rainNumLen * 6 * 3;
  int rainNumRight  = 235; // right edge for both symbol and text
  int rainNumX      = rainNumRight - rainNumWidth;
  int rainIconWidth = 30;                           // width of the bitmap
  int rainIconX     = rainNumX - rainIconWidth - 7; // 7px spacing
  tft.drawBitmap(rainIconX, 177, image_Chance_of_Rain_Symbol_bits, rainIconWidth, 32, WHITE);
  tft.setCursor(rainNumX, 183);
  tft.setTextColor(WHITE);
  tft.print(Chance_of_Rain_Number_text);

  // Below Humid Divider
  // tft.drawLine(5, 212, 234, 212, WHITE);
  tft.drawFastHLine(5, 212, 229, WHITE);

  // Time (centered)
  uartTimeoutFlag == true ? drawTextCentered(Time_text, 227, 4, RED) : drawTextCentered(Time_text, 227, 4, WHITE);

  // Week_Month_Day (centered) E.g. "Monday - Jan 17th"
  drawTextCentered(Week_Month_Day_text, 275, 2, WHITE);

  // Date Underline
  // tft.drawLine(20, 294, 219, 294, WHITE);
  tft.drawFastHLine(20, 294, 200, WHITE); // Centered underline

  // Date (centered) E.g. "1/17/2025"
  drawTextCentered(Date_text, 301, 2, WHITE);
  tft.setTextWrap(false);

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
  int len       = strlen(text);
  int textWidth = len * 6 * textSize;
  int x         = (240 - textWidth) / 2;
  tft.setTextColor(color);
  tft.setCursor(x, y);
  tft.print(text);
}

// Draw text aligned left, right, or center on the display
// @param text      The string to print
// @param y         The y position (top of text)
// @param textSize  The text size (as used by setTextSize)
// @param color     The 16-bit color value
// @param align     Alignment: "left", "right", or "center"
void drawTextAligned(const char *text, int y, int textSize, uint16_t color, const char *align) {
  tft.setTextSize(textSize);
  int len       = strlen(text);
  int textWidth = len * 6 * textSize;
  int x         = 0;
  if (strcmp(align, "left") == 0) {
    x = 0;
  } else if (strcmp(align, "right") == 0) {
    x = 240 - textWidth;
  } else if (strcmp(align, "center") == 0) {
    x = (240 - textWidth) / 2;
  }
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
    int    sep   = data.indexOf(',', idx);
    String pair  = (sep == -1) ? data.substring(idx) : data.substring(idx, sep);
    int    colon = pair.indexOf(':');
    if (colon > 0) {
      String key   = pair.substring(0, colon);
      String value = pair.substring(colon + 1);
      value.trim();
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

  int tempLen    = strlen(tempStr);
  int charWidth  = 6 * textSize; // Each char is 6px wide in Adafruit_GFX
  int totalWidth = tempLen * charWidth;
  int totalHeight = textSize * 7; // Height in pixels based on text size

  // Draw the temperature string
  tft.setTextSize(textSize);
  tft.setTextColor(color, BLACK);
  tft.setCursor(x, y);
  tft.print(tempStr);

  // Calculate where to draw the degree symbol
  int degRadius = 3; // Default
  uint8_t F_size = 2; // Default size for 'F'
  switch (textSize) {
    case 3: degRadius = 3; F_size = 1; break;
    case 4: degRadius = 4; F_size = 1; break;
    case 5: degRadius = 5; F_size = 2; break;
    case 6: degRadius = 5; F_size = 2; break;
    case 7: degRadius = 5; F_size = 2; break;
  }

  int degX = x + totalWidth + degRadius; // Small gap after temp
  int degY = y + degRadius;             // Vertically aligned with text

  tft.drawCircle(degX, degY, degRadius, WHITE);

  int F_X = x + totalWidth; // Small gap after temp
  int F_Y = (y + totalHeight) - (F_size * 7); // Bottom-aligned with temp text

  tft.setTextSize(F_size);
  tft.setTextColor(DARKGREY, BLACK);

  tft.setCursor(F_X, F_Y);
  tft.print('F');
}

// Helper function to draw the correct weather icon based on Cloud_text
void drawWeatherIconForCloudText() {
  String cloud = String(Cloud_text);
  cloud.toLowerCase(); // Convert to lowercase for matching
  // Rainy
  if (cloud.indexOf("rain") != -1 || cloud.indexOf("rainy") != -1 || cloud.indexOf("pouring") != -1 || cloud.indexOf("hail") != -1) {
    tft.drawBitmap(8, 10, image_weather_raining_bits, 85, 80, 0xFFFF);
  }
  // Lightning/Storm
  else if (cloud.indexOf("lightning") != -1 || cloud.indexOf("lightning-rainy") != -1 || cloud.indexOf("thunder") != -1) {
    tft.drawBitmap(10, 10, image_weather_lightning_bits, 90, 85, 0xFFFF);
  }
  // Snow
  else if (cloud.indexOf("snowy") != -1 || cloud.indexOf("snowy-rainy") != -1) {
    tft.drawBitmap(9, 6, image_weather_snow_bits, 90, 96, 0xFFFF);
  }
  // Cloudy + Sunny
  else if ((cloud.indexOf("partlycloudy") != -1 && cloud.indexOf("cloud") != -1) || cloud.indexOf("cloudy-sunny") != -1) {
    tft.drawBitmap(10, 10, image_weather_cloudy_sunny_bits, 85, 80, 0xFFFF);
  }
  // Cloudy
  else if (cloud.indexOf("cloudy") != -1 || cloud.indexOf("fog") != -1 || cloud.indexOf("windy") != -1 || cloud.indexOf("windy-variant") != -1) {
    tft.drawBitmap(10, 20, image_weather_cloud_bits, 85, 80, 0xFFFF);
  }
  // Exceptional/Unknown
  else if (cloud.indexOf("exceptional") != -1 || cloud.indexOf("unknown") != -1) {
    tft.drawBitmap(10, 6, image_weather_exceptional_bits, 90, 90, 0xFFFF);
  }
  // Night/Moon
  else if (cloud.indexOf("clear-night") != -1 || cloud.indexOf("moon") != -1 || cloud.indexOf("night") != -1) {
    tft.drawBitmap(10, 15, image_weather_moon_bits, 80, 80, 0xFFFF);
  }
  // Sunny/Clear
  else if (cloud.indexOf("sunny") != -1 || cloud.indexOf("clear") != -1) {
    tft.drawBitmap(5, 5, image_weather_sunny_bits, 90, 96, 0xFFFF);
  }
  // Default fallback
  else {
    tft.drawBitmap(10, 20, image_weather_cloud_bits, 85, 80, 0xFFFF);
    minorErrorFlag = true; // Set minor error flag for unrecognized cloud text
    Serial.println("Warning: Unrecognized cloud text: " + cloud);
  }
}
