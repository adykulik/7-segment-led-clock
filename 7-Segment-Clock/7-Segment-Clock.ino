#include <Wire.h>
#include <RtcDS3231.h>                        // Include RTC library by Makuna: https://github.com/Makuna/Rtc
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <FastLED.h>
#include <FS.h>                               // Please read the instructions on http://arduino.esp8266.com/Arduino/versions/2.3.0/doc/filesystem.html#uploading-files-to-file-system
#define countof(a) (sizeof(a) / sizeof(a[0]))
#define NUM_LEDS 86                           // Total of 86 LED's     
#define DATA_PIN D6                           // Change this if you are using another type of ESP board than a WeMos D1 Mini
#define MILLI_AMPS 2400 

#define WIFIMODE 2                            // 0 = Only Soft Access Point, 1 = Only connect to local WiFi network with UN/PW, 2 = Both

#if defined(WIFIMODE) && (WIFIMODE == 0 || WIFIMODE == 2)
  const char* APssid = "CLOCK_AP";        
  const char* APpassword = "1234567890";  
#endif
  
#if defined(WIFIMODE) && (WIFIMODE == 1 || WIFIMODE == 2)
  #include "Credentials.h"                    // Create this file in the same directory as the .ino file and add your credentials (#define SID YOURSSID and on the second line #define PW YOURPASSWORD)
  const char *ssid = SID;
  const char *password = PW;
#endif

RtcDS3231<TwoWire> Rtc(Wire);
ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdateServer;
CRGB LEDs[NUM_LEDS];

// Settings
unsigned long prevTime = 0;
bool autoCheck = false;
byte r_val = 255;
byte g_val = 0;
byte b_val = 0;
unsigned long lastModeSwitch = 0;
byte clockMode = 0;  // 0 for clock, 1 for date, 2 for temperature
unsigned long switchInterval = 7000; // Time interval for switching modes in milliseconds
bool dotsOn = true;
byte brightness = 255;
float temperatureCorrection = -3.0;
byte temperatureSymbol = 12;                  // 12=Celcius, 13=Fahrenheit check 'numbers'
byte hourFormat = 24;                         // Change this to 12 if you want default 12 hours format instead of 24               
// define as AI LIGHT_SENSOR_PIN
long numbers[] = {
  0b000111111111111111111,  // [0] 0
  0b000111000000000000111,  // [1] 1
  0b111111111000111111000,  // [2] 2
  0b111111111000000111111,  // [3] 3
  0b111111000111000000111,  // [4] 4
  0b111000111111000111111,  // [5] 5
  0b111000111111111111111,  // [6] 6
  0b000111111000000000111,  // [7] 7
  0b111111111111111111111,  // [8] 8
  0b111111111111000111111,  // [9] 9
  0b000000000000000000000,  // [10] off
  0b111111111111000000000,  // [11] degrees symbol
  0b000000111111111111000,  // [12] C(elsius)
  0b111000111111111000000,  // [13] F(ahrenheit)
};

void setup() {
  Serial.begin(115200); 
  delay(200);

  // RTC DS3231 Setup
  Rtc.Begin();    
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);

  if (!Rtc.IsDateTimeValid()) {
      if (Rtc.LastError() != 0) {
          // we have a communications error see https://www.arduino.cc/en/Reference/WireEndTransmission for what the number means
          Serial.print("RTC communications error = ");
          Serial.println(Rtc.LastError());
      } else {
          // Common Causes:
          //    1) first time you ran and the device wasn't running yet
          //    2) the battery on the device is low or even missing
          Serial.println("RTC lost confidence in the DateTime!");
          // following line sets the RTC to the date & time this sketch was compiled
          // it will also reset the valid flag internally unless the Rtc device is
          // having an issue
          Rtc.SetDateTime(compiled);
      }
  }

  WiFi.setSleepMode(WIFI_NONE_SLEEP);  

  delay(200);
  //Serial.setDebugOutput(true);

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(LEDs, NUM_LEDS);  
  FastLED.setDither(false);
  FastLED.setCorrection(TypicalLEDStrip);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, MILLI_AMPS);
  fill_solid(LEDs, NUM_LEDS, CRGB::Black);
  FastLED.show();

  // WiFi - AP Mode or both
#if defined(WIFIMODE) && (WIFIMODE == 0 || WIFIMODE == 2) 
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(APssid, APpassword);    // IP is usually 192.168.4.1
  Serial.println();
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
#endif

  // WiFi - Local network Mode or both
#if defined(WIFIMODE) && (WIFIMODE == 1 || WIFIMODE == 2) 
  byte count = 0;
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    // Stop if cannot connect
    if (count >= 60) {
      Serial.println("Could not connect to local WiFi.");      
      return;
    }
       
    delay(500);
    Serial.print(".");
    LEDs[count] = CRGB::Green;
    FastLED.show();
    count++;
  }
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());

  IPAddress ip = WiFi.localIP();
  Serial.println(ip[3]);
#endif   

  httpUpdateServer.setup(&server);

  // Handlers
  server.on("/color", HTTP_POST, []() {    
    r_val = server.arg("r").toInt();
    g_val = server.arg("g").toInt();
    b_val = server.arg("b").toInt();
    server.send(200, "text/json", "{\"result\":\"ok\"}");
  });
  
  server.on("/autoCheck", HTTP_POST, []() {
  String autoCheckValue = server.arg("autoCheck");
  autoCheck = (autoCheckValue == "true"); // Correctly sets autoCheck as a boolean

  server.send(200, "text/json", "{\"result\":\"ok\"}");
});


  server.on("/setdate", HTTP_POST, []() { 
    // Sample input: date = "Dec 06 2009", time = "12:34:56"
    // Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec
    String datearg = server.arg("date");
    String timearg = server.arg("time");
    Serial.println(datearg);
    Serial.println(timearg);    
    char d[12];
    char t[9];
    datearg.toCharArray(d, 12);
    timearg.toCharArray(t, 9);
    RtcDateTime compiled = RtcDateTime(d, t);
    Rtc.SetDateTime(compiled);   
    clockMode = 0;     
    server.send(200, "text/json", "{\"result\":\"ok\"}");
  });

  server.on("/brightness", HTTP_POST, []() {    
    brightness = server.arg("brightness").toInt();    
    server.send(200, "text/json", "{\"result\":\"ok\"}");
  });
  
  server.on("/temperature", HTTP_POST, []() {   
    temperatureCorrection = server.arg("correction").toInt();
    temperatureSymbol = server.arg("symbol").toInt();
    clockMode = 1;     
    server.send(200, "text/json", "{\"result\":\"ok\"}");
  });  

  server.on("/hourformat", HTTP_POST, []() {   
    hourFormat = server.arg("hourformat").toInt();
    clockMode = 0;     
    server.send(200, "text/json", "{\"result\":\"ok\"}");
  }); 

  server.on("/clock", HTTP_POST, []() {       
    clockMode = 0;     
    server.send(200, "text/json", "{\"result\":\"ok\"}");
  });  
  
  // Before uploading the files with the "ESP8266 Sketch Data Upload" tool, zip the files with the command "gzip -r ./data/" (on Windows I do this with a Git Bash)
  // *.gz files are automatically unpacked and served from your ESP (so you don't need to create a handler for each file).
  server.serveStatic("/", SPIFFS, "/", "max-age=86400");
  server.begin();     

  SPIFFS.begin();
  Serial.println("SPIFFS contents:");
  Dir dir = SPIFFS.openDir("/");
  while (dir.next()) {
    String fileName = dir.fileName();
    size_t fileSize = dir.fileSize();
    Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), String(fileSize).c_str());
  }
  Serial.println(); 
  
}

void loop() {
  server.handleClient(); 
  unsigned long currentMillis = millis();  

  // Mode switching logic
  if (currentMillis - lastModeSwitch >= switchInterval) {
    clockMode = (clockMode == 0) ? 1 : 0; // switching between time and date
    lastModeSwitch = currentMillis;
  }

  /*
  if (currentMillis - lastModeSwitchTime >= switchInterval) {
    clockMode = (clockMode + 1) % 3; // switching between time date and temperature
    lastModeSwitchTime = currentMillis;
  }
  */

  // Update display every second
  if (currentMillis - prevTime >= 1000) {
    prevTime = currentMillis;

    // Adjust brightness based on autoCheck
    if (autoCheck) {
      int sensorValue = analogRead(LIGHT_SENSOR_PIN); // Replace with your light sensor pin
      int dynamicBrightness = map(sensorValue, 0, 1023, 0, 255); // Adjust range as needed
      FastLED.setBrightness(dynamicBrightness);
    } else {
      FastLED.setBrightness(brightness);
    }

    // Update display based on the current mode
    if (clockMode == 0) {
      updateClock();
    } else if (clockMode == 1) {
      updateDate();
    } else if (clockMode == 2) {
      updateTemperature();
    }

    FastLED.show();
  }   
}



void displayNumber(byte number, byte segment, CRGB color) {
  /*
   * 
      __ __ __        __ __ __          __ __ __        12 13 14  
    __        __    __        __      __        __    11        15
    __        __    __        __      __        __    10        16
    __        __    __        __  42  __        __    _9        17
      __ __ __        __ __ __          __ __ __        20 19 18  
    __        65    __        44  43  __        21    _8        _0
    __        __    __        __      __        __    _7        _1
    __        __    __        __      __        __    _6        _2
      __ __ __       __ __ __           __ __ __       _5 _4 _3   

   */
 
  // segment from left to right: 3, 2, 1, 0
  byte startindex = 0;
  switch (segment) {
    case 0:
      startindex = 0;
      break;
    case 1:
      startindex = 21;
      break;
    case 2:
      startindex = 44;
      break;
    case 3:
      startindex = 65;
      break;    
  }

  for (byte i=0; i<21; i++){
    yield();
    LEDs[i + startindex] = ((numbers[number] & 1 << i) == 1 << i) ? color : alternateColor;
  } 
}

void allBlank() {
  for (int i=0; i<NUM_LEDS; i++) {
    LEDs[i] = CRGB::Black;
  }
  FastLED.show();
}

void updateHours(int hour) {
    byte h1 = hour / 10;
    byte h2 = hour % 10;
    CRGB color = CRGB(r_val, g_val, b_val);

    if (h1 > 0)
        displayNumber(h1, 3, color);
    else 
        displayNumber(10, 3, color);  // Blank

    displayNumber(h2, 2, color);
}

void updateMinutes(int mins) {
    byte m1 = mins / 10;
    byte m2 = mins % 10;
    CRGB color = CRGB(r_val, g_val, b_val);

    displayNumber(m1, 1, color);
    displayNumber(m2, 0, color);
}

void updateClock() {  
    RtcDateTime now = Rtc.GetDateTime();
    int hour = now.Hour();
    int mins = now.Minute();

    if (hourFormat == 12 && hour > 12)
        hour = hour - 12;

    updateHours(hour);
    updateMinutes(mins);
    displayDots(CRGB(r_val, g_val, b_val));  
}

void updateDate() {
  RtcDateTime now = Rtc.GetDateTime();

  byte day = now.Day();
  byte month = now.Month();

  CRGB color = CRGB(r_val, g_val, b_val);

  // Display the day (DD)
  displayNumber(day / 10, 3, color);   // Tens place of day
  displayNumber(day % 10, 2, color);   // Ones place of day

  // Display the month (MM)
  displayNumber(month / 10, 1, color); // Tens place of month
  displayNumber(month % 10, 0, color); // Ones place of month
}


void displayDots(CRGB color) {
  if (dotsOn) {
    LEDs[42] = color;
    LEDs[43] = color;
  } else {
    LEDs[42] = CRGB::Black;
    LEDs[43] = CRGB::Black;
  }

  dotsOn = !dotsOn;  
}

void hideDots() {
  LEDs[42] = CRGB::Black;
  LEDs[43] = CRGB::Black;
}

void updateTemperature() {
  RtcTemperature temp = Rtc.GetTemperature();
  float ftemp = temp.AsFloatDegC();
  float ctemp = ftemp + temperatureCorrection;
  Serial.print("Sensor temp: ");
  Serial.print(ftemp);
  Serial.print(" Corrected: ");
  Serial.println(ctemp);

  if (temperatureSymbol == 13)
    ctemp = (ctemp * 1.8000) + 32;

  byte t1 = int(ctemp) / 10;
  byte t2 = int(ctemp) % 10;
  CRGB color = CRGB(r_val, g_val, b_val);
  displayNumber(t1,3,color);
  displayNumber(t2,2,color);
  displayNumber(11,1,color);
  displayNumber(temperatureSymbol,0,color);
  hideDots();
}

void printDateTime(const RtcDateTime& dt)
{
    char datestring[20];

    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
            dt.Month(),
            dt.Day(),
            dt.Year(),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );
    Serial.println(datestring);
}
