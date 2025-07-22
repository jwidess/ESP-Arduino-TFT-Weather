# ESP-Arduino-TFT-Weather
This project is a simple weather display for utilizing an old MCUFRIEND_kbv 320x240 ILI9341 2.4" TFT Display shield with an Arduino Uno for display driving and an ESP8266 for data collection and Home Assistant Integration.

## Why does this exist?
I wanted to make use of some old hardware that had been collecting dust in my lab. This old 320x240 2.4" TFT Display shield and a handful of Arduino Uno's I'd never end up using, due to todays affordability of more powerful devices, made me realize I had to find something to use these for, otherwise they where going to get e-wasted. The display shield I am using requires both an SPI connection and an 8-bit parallel connection, and if I wanted to use that along with the touchscreen and SD Card interfaces, an ESP wouldn't have enough GPIOs. On-top of this it would be a wiring nightmare! So I decided to stick with using the display shield, well as a shield, and offload the weather data acquisition to a WiFi capable device like the ESP8266. With this setup I could make use of this display, an Arduino Uno and an ESP8266, all of which are becoming fossils in this day and age.

## Parts List:
- **Electronics:**
  - [MCUFRIEND_kbv 320x240 ILI9341 2.4" TFT Display Shield](https://www.aliexpress.us/item/3256808536298899.html) *(Other varieties may work)*
  - Arduino Uno
  - ESP8266 *(Or ESP32, really anything that runs ESPHome)*
  - [USB-C Breakout](https://www.sparkfun.com/sparkfun-usb-c-breakout.html)
- **3D Printed Parts:** (PLA works fine)
  - Enclosure Main Body
  - Top Lid
  - Bottom Lid
- **Screws:**
  - 4x M3x10mm Screws (Lids)
  - 2x M3x6mm Screws (USB-C Breakout)

## Example Images:
![Front of Device](https://github.com/jwidess/ESP-Arduino-TFT-Weather/blob/main/finished_front.jpg?raw=true)
![Rear of Device](https://github.com/jwidess/ESP-Arduino-TFT-Weather/blob/main/finished_rear.jpg?raw=true)

## Full Image Album Here: https://photos.app.goo.gl/iTXBMTu8txQHoKQs9

## Home Assistant & ESPHome
ESPHome acts as a bridge between Home Assistant and the Arduino weather display. Home Assistant gathers weather data from various sources, formats it using templates, and sends it to the ESP8266 device running ESPHome. The ESP8266 then transmits this data via UART to the Arduino, which updates the TFT display.

**How it works:**
- Home Assistant collects weather data (temperature, humidity, rain chance, condition, etc.) from multiple integrations and sensors. (These will need to be customized however you like, I've provided my `.yaml` files as examples)
- Custom templates in `templates.yaml` format the data into a single UART payload string.
- ESPHome receives this payload as a text sensor and sends it to the Arduino over UART every minute (and on power-up).
- The Arduino parses the UART message and updates the display with the latest weather information.
- Display icons and values are mapped according to the weather condition received from Home Assistant.
- Power to the Arduino and thus display can be controlled via Home Assistant using the ESPHome switch entity. I have pin D6 (GPIO12) connected to a MOSFET to control power. Refer to [Wiring Schematics](#Wiring-Schematic)


## Wiring Schematic:
A screenshot of the simple wiring schematic can be found below,

![Wiring Schematic Image](https://github.com/jwidess/ESP-Arduino-TFT-Weather/blob/main/Schematics/schematic_image.png?raw=true)