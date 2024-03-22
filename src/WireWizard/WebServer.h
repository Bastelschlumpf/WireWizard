/*
   Copyright (C) 2024 SFini

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

/**
  * @file WebServer.h
  *
  * Webserver Interface to communicate via esp32 in station or access-point mode. 
  * Works with the SPIFFS (.html, .css, .js).
  */

#include <WiFi.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include "EventQueue.h"


/**
  * My Webserver interface. Works together with .html, .css and .js files from the SPIFFS.
  */
class MyWebServer
{
protected:
  AsyncWebServer server;

private:
  String WifiGetRssiAsQuality(int rssi);
  
  void   handleRoot       (AsyncWebServerRequest *request);
  void   handleMain       (AsyncWebServerRequest *request);
  void   handlePushButton (AsyncWebServerRequest *request);
  void   handleNotFound   (AsyncWebServerRequest *request);

public:
  MyWebServer();
  
  bool begin();
};

/* ******************************************** */

/** Constructor/Destructor */
MyWebServer::MyWebServer()
  : server(80)
{
}

/** Conversion of the RSSI value to a quality value. */
String MyWebServer::WifiGetRssiAsQuality(int rssi)
{
  int quality = 0;

  if (rssi <= -100) {
    quality = 0;
  } else if (rssi >= -50) {
    quality = 100;
  } else {
    quality = 2 * (rssi + 100);
  }
  return String(quality);
}

/** Starts the Webserver in station and/or ap mode and sets all the callback functions for the specific urls. 
  * If you use rtc yourself you have to switch off the automatic Wifi configuration with WiFi.persistent(false)
  * (because it uses also the RTC memory) otherwise the WIFI won't start after a Deep Sleep.
  */
bool MyWebServer::begin()
{
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return false;
  }
    
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SID, WIFI_PW);

  // Try 30 seconds
  for (int i = 0; i < 60 && WiFi.status() != WL_CONNECTED; i++) { 
    Serial.print(F("."));
    delay(500);
  }
  Serial.println();

  WiFi.softAP(SOFT_AP_NAME, SOFT_AP_PW);
  String softAPIP         = WiFi.softAPIP().toString();
  String softAPmacAddress = WiFi.softAPmacAddress();
  
  Serial.println((String) "Soft AP: " + SOFT_AP_NAME + " (" + softAPIP + ") (" + softAPmacAddress + ")");
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println((String) "Connected to " + WiFi.localIP() + "SSID (RSSI): " + WIFI_SID + " (" + WifiGetRssiAsQuality(WiFi.RSSI()) + "%)");
    Serial.println("Connected");
  } else {
    Serial.println("No connection! Try AP!");
  }

  server.on("/",          HTTP_GET, [this](AsyncWebServerRequest *request) { this->handleRoot(request);       });
  server.on("/Main.html", HTTP_GET, [this](AsyncWebServerRequest *request) { this->handleMain(request);       });
  server.on("/button",    HTTP_GET, [this](AsyncWebServerRequest *request) { this->handlePushButton(request); });
  server.onNotFound(                [this](AsyncWebServerRequest *request) { this->handleNotFound(request);   }); 
  
  server.begin(); 
  return true;
}

/** Redirect a root call to the Main.html site. */
void MyWebServer::handleRoot(AsyncWebServerRequest *request)
{
  Serial.println("handleRoot");
  
  request->redirect("Main.html");
}

/** Sends the Main.html to the client. */
void MyWebServer::handleMain(AsyncWebServerRequest *request)
{
  Serial.println("handleMain");
  
  if (SPIFFS.exists("/Main.html")){
    request->send(SPIFFS, "/Main.html", "text/html");
  } else {
    handleNotFound(request);
  }
}

/** Default for an unknown web request on not found. */
void MyWebServer::handleNotFound(AsyncWebServerRequest *request) 
{
  Serial.println("handleNotFound");

  String message = F("File Not Found\n\n");

  message += F("URI: ");
  message += request->url();
  message += F("\nMethod: ");
  message += (request->method() == HTTP_GET) ? "GET" : "POST";
  message += F("\nArguments: ");
  message += request->args();
  message += F("\n");

  for(int i=0; i<request->args(); i++){
    message += " " + request->argName(i) + ": " + request->arg(i) + "\n";
  }
  
  if (SPIFFS.exists("/Main.html")){
    request->send(SPIFFS, "/Main.html", "text/html");
  } else {
    request->send(404, "text/plain", message);
  }
}

/** Handle the pushButton Event */
void MyWebServer::handlePushButton(AsyncWebServerRequest *request)
{
  Serial.println("handlePushButton");

  if (!request->hasArg("name")) {
    request->send(400, "text/plain", "Option name not found!");
  } else {
    static bool enabled = true;
    
    Event  event;
    String buttonName  = request->arg("name");
    String buttonValue = "";
    String response    = "Ok";

    Serial.println("Name:  " + buttonName);
    if (request->hasArg("value")) {
      buttonValue = request->arg("value");
      Serial.println("Value: " + buttonValue);
    }
  
         if (buttonName == "XLeft")  event.pushButton = Event::X_LEFT;
    else if (buttonName == "XRight") event.pushButton = Event::X_RIGHT;
    else if (buttonName == "YLeft")  event.pushButton = Event::Y_LEFT;
    else if (buttonName == "YRight") event.pushButton = Event::Y_RIGHT;
    else if (buttonName == "ZLeft")  event.pushButton = Event::Z_LEFT;
    else if (buttonName == "ZRight") event.pushButton = Event::Z_RIGHT;
    else if (buttonName == "OnOff") {
      enabled = !enabled;
      if (enabled) {
        event.pushButton = Event::ON;
        response = "On";
      } else {
        event.pushButton = Event::OFF;
        response = "Off";
      }
    } else if (buttonName == "Steps") {
      event.pushButton = Event::STEPS;
      event.steps      = atoi(buttonValue.c_str());
    } else if (buttonName == "Speed") {
      event.pushButton = Event::SPEED;
      event.speed      = atoi(buttonValue.c_str());
    }
    eventQueue.send(event);
    
    request->send(200, "text/html", response);    
  }
}
