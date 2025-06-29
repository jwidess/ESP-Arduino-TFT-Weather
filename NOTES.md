# Project Notes!

## Display Testing Results
**Display Name on AliExpress:** 2.4inch TFT LCD Display Module ILI9341 Resistive Touchscreen 8-bit Parallel Interface SD Card Slot for Arduino - AliExpress
Using `graphictest_kbv.ino` had by far the best performance compared to other display libraries. Everything was drawn very quickly. 
### Touchscreen Calibration
From `MCUFRIEND_kbv`->`TouchScreen_Calibr_native`
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