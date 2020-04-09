//
// Author: Mohsin Jessa
// STEAMetics
// Date: March 29, 2020
// 
// Adapted from: https:\//www.youtube.com/watch?v=3Vf-nX3JOTQ
//

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//#include <Fonts/FreeMono9pt7b.h> // I decided to use the default fonts

//Connect the SCL pin of OLED to D1 of ESP8266-NodeMCU
//Connect the SDA pin of OLED to D2 of ESP8266-NodeMCU

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const char* ssid = "your_wifi_ssid";  //SSID of your WiFi
const char* password = "wifi_password";  //WiFi PASSWORD
WiFiClient client;

// Website for the API request (Here it is www.thingspeak.com)
const char* host = "api.thingspeak.com";
const int httpPortRead = 80;
// "XXXXXXXXXXXXXXX" is the API Key assigned by Thingspeak i.e ThingHTTP
const char* url = "/apps/thinghttp/send_request?api_key=xxxxxxxx_api#1"; // Total People Tested
const char* url2 = "/apps/thinghttp/send_request?api_key=xxxxxxxx_api#2"; // Confirmed Cases Positive
const char* url3 = "/apps/thinghttp/send_request?api_key=xxxxxxxx_api#3"; // Deaths
const char* url4 = "/apps/thinghttp/send_request?api_key=xxxxxxxx_api#4"; // Time Stamp of the data

HTTPClient http;

void setup() {

  Serial.begin(115200); // define the serial baud rate

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // I2C Address of the 128x64 OLED screen
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();  // clear the oled screen
  delay(3000);

  display.setTextSize(1);  // set the text font size
  display.setTextColor(WHITE); // select the text color - my oled supports only one color
  display.setCursor(0, 10); // decide where the text will be written
  display.println("Canada - Covid Stats");
  display.display();
  Serial.println("Canada - Covid Stata");

  WiFi.disconnect(); // make a WiFi connection and display progress on console
  delay(3000);
  Serial.println();
  display.setCursor(0, 20);
  display.println("Checking Wifi...");
  display.display();
  Serial.println("Checking Wifi...");
//  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) // wait until connected
  {
    delay(500);
    Serial.print(".");
  }
  display.setCursor(0,30);
  display.println("Wifi connected");
  display.display();
  Serial.println("");
  Serial.println("Wifi connected");
  delay(3000);

  Serial.println("IP address: "); // IP is really not required but show it anyways
  Serial.println(WiFi.localIP());   //You can get IP address assigned to ESP
}

void loop()
{
  while (WiFi.status() != WL_CONNECTED) // make sure we are still connected to WiFi
  {
    WiFi.begin(ssid, password);
    Serial.println("Reconnecting to WiFi..");
    delay(3000);
  }
  display.setCursor(0, 40); 
  display.println("Geting Data");
  display.display();
  delay(3000);
  display.clearDisplay();

  display.drawRoundRect(0, 0, 128, 17, 2, WHITE); // draw a nice rounded box for title text
  display.display();
  display.setTextColor(WHITE);
  display.setCursor(5, 4);
  display.println("Canada-Covid19 Stats");

  if ( http.begin(host, httpPortRead, url)) // now start getting the data
  {
    int httpCode = http.GET();
    if (httpCode > 0)
    {
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
      {
        String payload = http.getString();
        Serial.print("Tested: ");
        Serial.println(payload);

        display.setCursor(0, 20);
        display.println("Tested: ");
        display.setCursor(60, 20);
        display.println(payload);
      }
      if ( http.begin(host, httpPortRead, url2))
      {
        int httpCode = http.GET();
        if (httpCode > 0)
        {
          if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
          {
            String payload = http.getString();
            Serial.print("Positive: ");
            Serial.println(payload);

            display.setCursor(0, 30);
            display.println("Positive:");
            display.setCursor(60, 30);
            display.println(payload);
          }
          if ( http.begin(host, httpPortRead, url3))
          {
            int httpCode = http.GET();
            if (httpCode > 0)
            {
              if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
              {
                String payload = http.getString();
                Serial.print("Deaths: ");
                Serial.println(payload);

                display.setCursor(0, 40);
                display.println("Deaths:");
                display.setCursor(60, 40);
                display.println(payload);
                display.display();
              }
            }
          }
          if ( http.begin(host, httpPortRead, url4))
          {
            int httpCode = http.GET();
            if (httpCode > 0)
            {
              if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
              {
                String payload = http.getString();
                Serial.print("Time Stamp: ");
                Serial.println(payload);

                display.setCursor(0, 47);
                display.println(payload);
                display.display();
              }
            }
          }
        }
      }
      else
      {
        Serial.println("Error in response");
      }
      http.end();  //Close connection
      delay(3600000);  //GET Data at every 1hr
      Serial.println("NEW DATA");
      display.clearDisplay();
    }
  }
}
