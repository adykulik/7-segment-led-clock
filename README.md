# Arduino LED Clock and Temperature Display

**Note:** This project is currently under active development. I will be updating and improving the code over time, so stay tuned for the latest enhancements!

## Overview
This repository contains the source code for an Arduino-based project that alternates between displaying the current time and ambient temperature using a LED matrix. The project utilizes an ESP8266 module for web server capabilities and an RTC (Real-Time Clock) module for timekeeping.

## Features
- **Clock Mode (Mode 0):** Displays the current time on the LED matrix.
- **Temperature Mode (Mode 1):** Shows the current temperature, read from the RTC's temperature sensor.
- **Automatic Mode Switching:** Alternates between clock and temperature display every x seconds.
- **Web Interface:** Adjust settings like LED brightness and color via a web interface.
- **Auto-Brightness:** Adjusts LED brightness automatically based on ambient light (requires a light sensor).

## Hardware Requirements
- ESP8266 Module (e.g., WeMos D1 Mini)
- LED Matrix (86 LEDs, I use WS2812B eco IP30)
- RTC Module (e.g., DS3231)
- Light Sensor (for auto-brightness feature, optional)

## Setup and Installation
1. **Hardware Setup:** Connect the LED matrix, RTC module, and (optionally) the light sensor to the ESP8266.
2. **Software Upload:** Flash the provided code to the ESP8266 using the Arduino IDE.
3. **Web Interface:** Access the web interface via the ESP8266's IP address to adjust settings.
