// TFT-Driver-Mk1
// https://github.com/jwidess/ESP-Arduino-TFT-Weather
// Author: Jwidess
// This sketch is for a 2.4" 320x240 TFT ILI9341 display with touch panel on an Arduino Uno.
// ==========================================================================

#include <Arduino.h>

#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;
#include <TouchScreen.h>
#include <SPI.h>
#include "bitmaps.h"

#define MINPRESSURE 200
#define MAXPRESSURE 1000

// Assign human-readable names to some common 16-bit color values:
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

// ALL Touch panels and wiring is DIFFERENT
// char *name = "2.4 TFT ILI9341";

// From calibration on 6/28/2025
const int XP = 8, XM = A2, YP = A3, YM = 9; // 240x320 ID=0x9341
const int TS_LEFT = 155, TS_RT = 856, TS_TOP = 90, TS_BOT = 845;
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

int pixel_x, pixel_y; // Touch_getXY() updates global vars
bool Touch_getXY(void)
{
  TSPoint p = ts.getPoint();
  pinMode(YP, OUTPUT); // restore shared pins
  pinMode(XM, OUTPUT);
  digitalWrite(YP, HIGH); // because TFT control pins
  digitalWrite(XM, HIGH);
  bool pressed = (p.z > MINPRESSURE && p.z < MAXPRESSURE);
  if (pressed)
  {
    pixel_x = map(p.x, TS_LEFT, TS_RT, 0, tft.width()); //.kbv makes sense to me
    pixel_y = map(p.y, TS_TOP, TS_BOT, 0, tft.height());
  }
  return pressed;
}

// Example weather data (replace with real data later)
float temperature = 23.5; // Celsius
int cloudCover = 75;      // Percent
int humidity = 60;        // Percent
String windDir = "NE";    // Wind direction

// Lopaka UI text variables (updated names)
const char *TempHIGH_text = "82F";
const char *TempLOW_text = "72F";
const char *CurrentTEMP_text = "74F";
const char *Chance_of_Rain_Number_text = "4%";
const char *Humidity_Number_text = "5%";
const char *Time_text = "10:15 AM";
const char *Week_Month_Day_text = "Monday - Jan 17th";
const char *Date_text = "1/17/2025";

// Draw text centered horizontally on the display
// @param text      The string to print
// @param y         The y position (top of text)
// @param textSize  The text size (as used by setTextSize)
// @param color     The 16-bit color value
void drawTextCentered(const char *text, int y, int textSize, uint16_t color)
{
  tft.setTextSize(textSize);
  int len = strlen(text);
  int textWidth = len * 6 * textSize;
  int x = (240 - textWidth) / 2;
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
void drawTextAligned(const char *text, int y, int textSize, uint16_t color, const char *align)
{
  tft.setTextSize(textSize);
  int len = strlen(text);
  int textWidth = len * 6 * textSize;
  int x = 0;
  if (strcmp(align, "left") == 0)
  {
    x = 0;
  }
  else if (strcmp(align, "right") == 0)
  {
    x = 240 - textWidth;
  }
  else if (strcmp(align, "center") == 0)
  {
    x = (240 - textWidth) / 2;
  }
  tft.setTextColor(color);
  tft.setCursor(x, y);
  tft.print(text);
}

void drawWeatherUI()
{
  // Outline
  tft.drawRect(0, 0, 240, 320, 0xF206);

  // Cloud Cover Symbol
  tft.drawBitmap(5, 5, image_Cloud_Cover_Symbol_bits, 90, 96, 0xFFFF);

  // TempHIGH (right aligned, 5px padding)
  tft.setTextColor(0xF206);
  tft.setTextSize(5);
  int tempHighLen = strlen(TempHIGH_text);
  int tempHighWidth = tempHighLen * 6 * 5;
  tft.setCursor(240 - tempHighWidth, 10);
  tft.print(TempHIGH_text);

  // TempLOW (right aligned, 5px padding)
  tft.setTextColor(0x55E);
  tft.setTextSize(5);
  int tempLowLen = strlen(TempLOW_text);
  int tempLowWidth = tempLowLen * 6 * 5;
  tft.setCursor(240 - tempLowWidth, 57);
  tft.print(TempLOW_text);

  // Above Temp Divider
  tft.drawLine(5, 104, 234, 104, WHITE);

  // CurrentTEMP
  // tft.setTextColor(WHITE);
  // tft.setTextSize(7);
  // tft.setCursor(61, 115);
  // tft.print(CurrentTEMP_text);
  drawTextCentered(CurrentTEMP_text, 115, 7, WHITE);

  // Below Temp Divider
  tft.drawLine(5, 173, 234, 173, WHITE);

  // Humidity Symbol
  tft.drawBitmap(5, 180, image_Humidity_Symbol_bits, 17, 24, WHITE);

  // Humidity Number
  tft.setTextSize(3);
  tft.setCursor(28, 182);
  tft.print(Humidity_Number_text);

  // Chance of Rain Symbol and Number (right-aligned together)
  tft.setTextSize(3);
  int rainNumLen = strlen(Chance_of_Rain_Number_text);
  int rainNumWidth = rainNumLen * 6 * 3;
  int rainNumRight = 235; // right edge for both symbol and text
  int rainNumX = rainNumRight - rainNumWidth;
  int rainIconWidth = 30;                       // width of the bitmap
  int rainIconX = rainNumX - rainIconWidth - 7; // 7px spacing
  tft.drawBitmap(rainIconX, 177, image_Chance_of_Rain_Symbol_bits, rainIconWidth, 32, WHITE);
  tft.setCursor(rainNumX, 183);
  tft.setTextColor(WHITE);
  tft.print(Chance_of_Rain_Number_text);

  // Below Humid Divider
  tft.drawLine(5, 212, 234, 212, WHITE);

  // Time (centered)
  drawTextCentered(Time_text, 227, 4, WHITE);

  // Week_Month_Day (centered) E.g. "Monday - Jan 17th"
  drawTextCentered(Week_Month_Day_text, 275, 2, WHITE);

  // Date Underline
  tft.drawLine(20, 294, 219, 294, WHITE);

  // Date (centered) E.g. "1/17/2025"
  drawTextCentered(Date_text, 301, 2, WHITE);
  tft.setTextWrap(false);
}

void setup(void)
{
  Serial.begin(9600);
  uint16_t ID = tft.readID();
  Serial.print("TFT ID = 0x");
  Serial.println(ID, HEX);
  if (ID == 0xD3D3)
    ID = 0x9486; // write-only shield
  tft.begin(ID);
  tft.setRotation(0); // PORTRAIT
  tft.fillScreen(BLACK);
  drawWeatherUI();
}

void loop(void)
{
  delay(1000); // Adjust delay as needed
  drawWeatherUI();
}
