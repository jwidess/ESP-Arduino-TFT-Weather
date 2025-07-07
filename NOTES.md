# Project Notes!
General unorganized project notes.

## To-Do:
- Use AccuWeather `sensor.home_condition_day_0` as scrolling text of the days weather summary.

## Display Testing Results
**Display Name:** 2.4inch 320x240 TFT LCD Display Module ILI9341 Resistive Touchscreen 8-bit Parallel Interface SD Card Slot for Arduino - AliExpress

Using the [MCUFRIEND_kbv](https://github.com/prenticedavid/MCUFRIEND_kbv) library had by far the best performance compared to other display libraries. Everything was drawn very quickly. 

### 16-bit RGB Color Picker
https://rgbcolorpicker.com/565

## Condition Value Mapping to Bitmaps
Below is a table of all 15 possible values of the `condition` parameter in HA and how they are mapped to the available bitmaps in the `bitmaps.h`

| Condition Value: | Bitmap Displayed: |
| ---------------- | ----------------- |
| clear-night      | Moon              |
| exceptional      | Exceptional       |
| cloudy           | cloudy            |
| sunny            | Sunny             |
| snowy-rainy      | Snow              |
| snowy            | Snow              |
| rainy            | Rainy             |
| pouring          | Rainy             |
| hail             | Rainy             |
| lightning-rainy  | Lightning         |
| lightning        | Lightning         |
| partlycloudy     | Cloudy Sunny      |
| windy-variant    | Cloudy            |
| windy            | Cloudy            |
| fog              | Cloudy            |

## UART Comms
Example ESP8266 Send: `HIGH:82,LOW:72,TEMP:74,RAIN:15,HUM:47,TIME:10:15 AM,DATE:Monday - Jan 17th,DATETXT:1/17/2025,CLOUD:clear\n`

### Tesing UART RX:
Formatted input and output test:

<details>
<summary>Serial Monitor Output:</summary>

```
---- Sent utf8 encoded message: "HIGH:82,LOW:72,TEMP:74,RAIN:15,HUM:47,TIME:10:15 AM,DATE:Monday - Jan 17th,DATETXT:1/17/2025,CLOUD:clear\n" ----
--- Weather Data ---
TempHIGH_text: 82
TempLOW_text: 72
CurrentTEMP_text: 74
Chance_of_Rain_Number_text: 15%
Humidity_Number_text: 47%
Time_text: 10:15 AM
Week_Month_Day_text: Monday - Jan 17th
Date_text: 1/17/2025
--------------------
---- Sent utf8 encoded message: "HIGH:115,LOW:-15,TEMP:54,RAIN:100,HUM:100,TIME:12:34 PM,DATE:Saturday - Feb 21st,DATETXT:12/11/2029,CLOUD:overcast\n" ----
--- Weather Data ---
TempHIGH_text: 115
TempLOW_text: -15
CurrentTEMP_text: 54
Chance_of_Rain_Number_text: 100%
Humidity_Number_text: 100%
Time_text: 12:34 PM
Week_Month_Day_text: Saturday - Feb 21st
Date_text: 12/11/2029
--------------------
```
</details>

### Touchscreen Calibration
From `MCUFRIEND_kbv`->`TouchScreen_Calibr_native`:
<details>
<summary>Serial Monitor Output:</summary>

```
Typical 30k Analog pullup with corresponding pin
would read low when digital is written LOW
e.g. reads ~25 for 300R X direction
e.g. reads ~30 for 500R Y direction

Testing : (A2, D8) = 25
Testing : (A3, D9) = 32
ID = 0x9341

cx=178 cy=132 cz=449  X, Y, Pressure
cx=182 cy=427 cz=490  X, Y, Pressure
cx=197 cy=806 cz=376  X, Y, Pressure
cx=472 cy=112 cz=687  X, Y, Pressure
cx=479 cy=832 cz=570  X, Y, Pressure
cx=824 cy=99 cz=713  X, Y, Pressure
cx=826 cy=438 cz=712  X, Y, Pressure
cx=833 cy=828 cz=628  X, Y, Pressure

*** COPY-PASTE from Serial Terminal:
const int XP=8,XM=A2,YP=A3,YM=9; //240x320 ID=0x9341
const int TS_LEFT=155,TS_RT=856,TS_TOP=90,TS_BOT=845;

PORTRAIT  CALIBRATION     240 x 320
x = map(p.x, LEFT=155, RT=856, 0, 240)
y = map(p.y, TOP=90, BOT=845, 0, 320)

LANDSCAPE CALIBRATION     320 x 240
x = map(p.y, LEFT=90, RT=845, 0, 320)
y = map(p.x, TOP=856, BOT=155, 0, 240)
```

Output of `LCD_ID_readnew.ino`:

```
Read Special Registers on MCUFRIEND UNO shield
controllers either read as single 16-bit
e.g. the ID is at readReg(0)
or as a sequence of 8-bit values
in special locations (first is dummy)

diagnose any controller
reg(0x0000) 00 00	ID: ILI9320, ILI9325, ILI9335, ...
reg(0x0004) 00 00 00 00	Manufacturer ID
reg(0x0009) 00 00 61 00 00	Status Register
reg(0x000A) 00 08	Get Power Mode
reg(0x000C) 00 06	Get Pixel Format
reg(0x0030) 00 00 00 00 00	PTLAR
reg(0x0033) 00 00 00 00 00 00 00	VSCRLDEF
reg(0x0061) 00 00	RDID1 HX8347-G
reg(0x0062) 00 00	RDID2 HX8347-G
reg(0x0063) 00 00	RDID3 HX8347-G
reg(0x0064) 00 00	RDID1 HX8347-A
reg(0x0065) 00 00	RDID2 HX8347-A
reg(0x0066) 00 00	RDID3 HX8347-A
reg(0x0067) 00 00	RDID Himax HX8347-A
reg(0x0070) 00 00	Panel Himax HX8347-A
reg(0x00A1) 00 00 00 00 00	RD_DDB SSD1963
reg(0x00B0) 00 00	RGB Interface Signal Control
reg(0x00B3) 00 00 1B 1B 1B	Frame Memory
reg(0x00B4) 00 02	Frame Mode
reg(0x00B6) 00 0A 82 27 04	Display Control
reg(0x00B7) 00 06	Entry Mode Set
reg(0x00BF) 00 00 00 00 00 00	ILI9481, HX8357-B
reg(0x00C0) 00 21 00 00 00 00 00 00 00	Panel Control
reg(0x00C1) 00 10 10 10	Display Timing
reg(0x00C5) 00 31	Frame Rate
reg(0x00C8) 00 00 00 00 00 00 00 00 00 00 00 00 00	GAMMA
reg(0x00CC) 00 72	Panel Control
reg(0x00D0) 00 00 00 00	Power Control
reg(0x00D1) 00 00 00 00	VCOM Control
reg(0x00D2) 00 00 00	Power Normal
reg(0x00D3) 00 00 93 41	ILI9341, ILI9488
reg(0x00D4) 00 00 00 00	Novatek
reg(0x00DA) 00 00	RDID1
reg(0x00DB) 00 00	RDID2
reg(0x00DC) 00 00	RDID3
reg(0x00E0) 00 0F 16 14 0A 0D 06 43 75 33 06 0E 00 0C 09 08	GAMMA-P
reg(0x00E1) 00 08 2B 2D 04 10 04 3E 24 4E 04 0F 0E 35 38 0F	GAMMA-N
reg(0x00EF) 00 03 80 02 02 02	ILI9327
reg(0x00F2) 00 02 02 02 02 02 02 02 02 02 02 02	Adjust Control 2
reg(0x00F6) 00 01 00 00	Interface Control
```
</details>


