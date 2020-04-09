//
// Author: Mohsin Jessa
// STEAMetics
// Date: April 8, 2020
//
// Additional features: Data is fetched when first powered on, after that the screen saver kicks in with "Press Button to update".
// and when the button is pressed, it shows the cached data if the last refresh of data was less than an hour ago. If more than one
// hour has elapsed - fresh data is fetched from source.
// I use ISR - Interrupt Service Routine to handle the "Pressing of the button".
//
// Code has been modularized for ease of maintenance
//
// Adapted from: https:\//www.youtube.com/watch?v=3Vf-nX3JOTQ
//
// Get all the required libraries
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// define how frequent we'll fetch data
unsigned long oneHour = 60 * 60 * 1000;
unsigned long timeNow = millis();
unsigned long lastUpdate = 0;
String payload_tested;
String payload_positive;
String payload_deaths;
String payload_timestamp;

// define button connection
const int button = 14; //push button is connected to Pin14/GPIO14 = D5 + the GND and 3V

// button press means time to get updated data
boolean getupdatedData = false;

// Setup the ISR - Interrupt Service Routine for the button
// Activated when Button is clicked. This will initiate getting updated data
ICACHE_RAM_ATTR void buttonClicked() {
  Serial.println("BUTTON Clicked!!!");
  getupdatedData = true;
}

//Connect the SCL pin of OLED to D1 of ESP8266-NodeMCU
//Connect the SDA pin of OLED to D2 of ESP8266-NodeMCU
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

//
// define SSID and Passwords for WiFi connection
const char* ssid = "xxx";  //SSID of your WiFi
const char* password = "xxx";  //WiFi PASSWORD
WiFiClient client;

// Website for the API request (Here it is www.thingspeak.com)
const char* host = "api.thingspeak.com";
const int httpPortRead = 80;
// api_key=XXXXXXXXXXXXXXX is the API Key assigned by Thingspeak i.e ThingHTTP
const char* url = "/apps/thinghttp/send_request?api_key=xxxxxxx"; // Total People Tested
const char* url2 = "/apps/thinghttp/send_request?api_key=xxxxxxx"; // Confirmed Cases Positive
const char* url3 = "/apps/thinghttp/send_request?api_key=xxxxxxx"; // Deaths
const char* url4 = "/apps/thinghttp/send_request?api_key=xxxxxxx"; // Time Stamp of the data

HTTPClient http;

void setup() {

  Serial.begin(115200); // define the serial baud rate

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // I2C Address of the 128x64 OLED screen
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  display.clearDisplay();  // clear the oled screen
  display.display();
  delay(500);
  display.setTextSize(1);  // set the text font size
  display.setTextColor(WHITE); // select the text color - my oled supports only one color

  // call a routine to setup WiFi connectivity
  connectWiFi();

  pinMode(button, INPUT_PULLUP); // declare push button as input and setup the PULLUP resistor
  // Define the Button pin with an ISR and set it to fire on RISING mode
  attachInterrupt(digitalPinToInterrupt(button), buttonClicked, RISING);
  // setup a random seed generator - to compute random x,y coordinates
  // for oled display of scrolling text
  randomSeed(analogRead(0));

  // initially get the data
  getUpdates();
  lastUpdate = millis();
  delay(10000);
}

void loop() {
  timeNow = millis();
  if (getupdatedData) {
    if ((timeNow - lastUpdate) > (oneHour)) {
      Serial.println("Updating Data");
      getUpdates (); // button was clicked - get updated data
      getupdatedData = false; // wait for the next click
      delay(10000); // pause to digest the data!!!
      lastUpdate = millis();
    }
    else {
      Serial.println(" Refreshing Data");
      display.clearDisplay();
      display.display();
      displayHeader();
      displayTested(payload_tested);
      displayPositive(payload_positive);
      displayDeaths(payload_deaths);
      displayTimestamp (payload_timestamp);
      delay(10000);
      getupdatedData = false;
    }
  }
  else {
    wait4Button(); // as the name suggests - wait for the next click
  }
}

void getUpdates() {
  delay(2000);
  display.clearDisplay();
  display.display();

  Serial.println("NEW DATA");
  while (WiFi.status() != WL_CONNECTED) // make sure we are still connected to WiFi
  {
    WiFi.begin(ssid, password);
    Serial.println("Reconnecting to WiFi..");
    delay(2000);
  }
  display.setCursor(0, 40);
  display.println("Geting Data");
  display.display();
  delay(500);
  display.clearDisplay();

  displayHeader();

  if ( http.begin(host, httpPortRead, url)) // now start getting the data
  {
    int httpCode = http.GET();
    if (httpCode > 0)
    {
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
      {
        payload_tested = http.getString();
        displayTested(payload_tested);
      }
      if ( http.begin(host, httpPortRead, url2))
      {
        int httpCode = http.GET();
        if (httpCode > 0)
        {
          if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
          {
            payload_positive = http.getString();
            displayPositive(payload_positive);
          }
          if ( http.begin(host, httpPortRead, url3))
          {
            int httpCode = http.GET();
            if (httpCode > 0)
            {
              if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
              {
                payload_deaths = http.getString();
                displayDeaths(payload_deaths);
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
                payload_timestamp = http.getString();
                displayTimestamp (payload_timestamp);
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
    }
    lastUpdate = millis();
  }
  else {
    display.clearDisplay();
    delay(1000);
    Serial.println("Press button to refresh data");
  }
}

void connectWiFi (void) {
  display.setCursor(0, 10); // decide where the text will be displlayed
  display.println("Canada - Covid Stats");
  display.display();
  Serial.println("Canada - Covid Stats");

  WiFi.disconnect(); // make a WiFi connection and display progress on console
  delay(2000);
  Serial.println();
  display.setCursor(0, 20);
  display.println("Checking Wifi...");
  display.display();
  Serial.println("Checking Wifi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) // wait until connected
  {
    delay(500);
    Serial.print(".");
  }
  display.setCursor(0, 30);
  display.println("Wifi connected");
  display.display();
  Serial.println("");
  Serial.println("Wifi connected");
  delay(1000);

  Serial.print("IP address: "); // IP is really not required but show it anyways
  Serial.println(WiFi.localIP());   //You can get IP address assigned to ESP
}

void wait4Button (void) {
  getupdatedData = false;
  display.setTextSize(1); // Draw 1X-scale text
  display.setTextColor(WHITE);
//  int x = random(0,129);
  int y = random(0,41);
//  display.setCursor(10, 10);
  display.clearDisplay();
  display.display();
  display.setCursor(0, y); // show text at random location - like screen saver  
//  display.setCursor(random(0, 129), random(0, 41)); // show text at random location - like screen saver
//  delay(100);
  display.print("Press\n");
  display.setCursor(0, y+8);  
//  delay(100);
  display.print("button to\n");
  display.setCursor(0, y+16);
//  delay(100);
  display.print("update\n");
//  delay(100);
  display.display();      // Show initial text

  // Scroll in various directions, pausing in-between:
  display.startscrollright(0x00, 0x0F);
  delay(2500);
  display.stopscroll();
  display.startscrollleft(0x00, 0x0F);
  delay(2500);
  display.stopscroll();
  delay(1000);
}

void displayHeader(void) {
  display.drawRoundRect(0, 0, 128, 17, 2, WHITE); // draw a nice rounded box for title text
  display.display();
  display.setTextColor(WHITE);
  display.setCursor(5, 4);
  display.println("Canada-Covid19 Stats");
}

void displayTested (String msg) {
  Serial.print("Tested: ");
  Serial.println(msg); //payload_tested);

  display.setCursor(0, 20);
  display.println("Tested: ");
  display.setCursor(60, 20);
  display.println(msg); //payload_tested);
}

void displayPositive (String msg) {
  Serial.print("Positive: ");
  Serial.println(msg); //payload_positive);

  display.setCursor(0, 30);
  display.println("Positive:");
  display.setCursor(60, 30);
  display.println(msg); //payload_positive);
}

void displayDeaths (String msg) {
  Serial.print("Deaths: ");
  Serial.println(msg); //payload_deaths);

  display.setCursor(0, 40);
  display.println("Deaths:");
  display.setCursor(60, 40);
  display.println(msg) ;//payload_deaths);
  display.display();
}

void displayTimestamp (String msg) {
  Serial.print("Time Stamp: ");
  Serial.println(msg); //payload_timestamp);

  display.setCursor(0, 47);
  display.println(msg); //payload_timestamp);
  display.display();
}
