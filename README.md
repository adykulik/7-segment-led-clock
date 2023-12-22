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

## Hardware requirements

| Components                    |       |
| -------------                 | ----- |
|ESP8266 WeMos Mini D1          | 1x    |
|DS3231 RTC                     | 1x    |
|Micro USB Breakout board       | 1x    |
|Micro USB cable                | 1x    |
|Analog Output light sensor     | 1x    |
|5V 2.5A power supply           | 1x    |
|WS2812BECO IP30 LED Strip 60 LED'S per meter | 2m    |

## Setup and Installation
1. **Hardware Setup:** Connect the LED matrix, RTC module, and (optionally) the light sensor to the ESP8266.
2. **Software Upload:** Flash the provided code to the ESP8266 using the Arduino IDE.
3. **Web Interface:** Access the web interface via the ESP8266's IP address to adjust settings.


## License

<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="Creative Commons License" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><br />This work is licensed under a <a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License</a>.

As per the license requirements:
- **Attribution:** Appropriate credit is given to the original authors of the components used, along with a link to the CC BY-NC-SA 4.0 license.
- **NonCommercial:** The material is not used for commercial purposes.
- **ShareAlike:** If you remix, transform, or build upon the material, you must distribute your contributions under the same license as the original.

### Original Project
Parts of this project were based on [7-Segment-Digital-Clock-V2](https://github.com/leonvandenbeukel/7-Segment-Digital-Clock-V2), licensed under the CC BY-NC-SA 4.0.

