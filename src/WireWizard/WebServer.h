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


#include <WebServer.h>
#include <DNSServer.h>
#include "Spiffs.h"
#include "HtmlTag.h"
#include "Utils.h"
#include "EventQueue.h"


/**
  * My Webserver interface. Works together with .html, .css and .js files from the SPIFFS.
  * Works mostly with static functions because of the server callback functions.
  */
class MyWebServer
{
public:
  static WebServer server;    //!< Webserver class.
  static IPAddress ip;        //!< Soft AP ip Address
  static DNSServer dnsServer; //!< Dns server
  static String    softAPIP;
  static String    softAPmacAddress;
  static String    stationIP;
  static String    wifiAP;

protected:
  static bool loadFromSpiffs  (String path);
  static void AddTableBegin   (String &info);
  static void AddTableTr      (String &info);
  static void AddTableTr      (String &info, String name, String value);
  static void AddTableEnd     (String &info);
  static bool GetOption       (String id, String &option);
  static bool GetOption       (String id, long   &option);
  static bool GetOption       (String id, double &option);
  static bool GetOption       (String id, bool   &option);
  static void AddBr           (String &info);
  static void AddOption       (String &info, String id, String name, bool value, bool addBr = true);
  static void AddOption       (String &info, String id, String name, String value, bool addBr = true, bool isPassword = false);
  static void AddIntervalInfo (String &info);

public:
  static void handleRoot();
  static void loadMain();
  static void handlePushButton();
  static void handleLoadMainInfo();
  static void handleNotFound();
  static void handleWebRequests();

public:
  bool isWebServerActive; //!< Is the webserver currently active.

public:
  MyWebServer();
  ~MyWebServer();
  
  bool begin();
  void handleClient();
};

/* ******************************************** */

IPAddress     MyWebServer::ip(192, 168, 1, 1);       
DNSServer     MyWebServer::dnsServer;
WebServer     MyWebServer::server(80);
String        MyWebServer::softAPIP;
String        MyWebServer::softAPmacAddress;
String        MyWebServer::stationIP;
String        MyWebServer::wifiAP;


/** Constructor/Destructor */
MyWebServer::MyWebServer()
  : isWebServerActive(false)
{
}

MyWebServer::~MyWebServer()
{
}

/** Starts the Webserver in station and/or ap mode and sets all the callback functions for the specific urls. 
  * If you use rtc yourself you have to switch off the automatic Wifi configuration with WiFi.persistent(false)
  * (because it uses also the RTC memory) otherwise the WIFI won't start after a Deep Sleep.
  */
bool MyWebServer::begin()
{
  wifiAP = WIFI_SID;
  
  WiFi.mode(WIFI_OFF); // workaround connection problem after deep sleep
  delay(100);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(SOFT_AP_NAME, SOFT_AP_PW);
  WiFi.softAPConfig(ip, ip, IPAddress(255, 255, 255, 0));  
  
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(53, F("*"), ip);
  softAPIP         = WiFi.softAPIP().toString();
  softAPmacAddress = WiFi.softAPmacAddress();
  Serial.println((String) F("SoftAPIP address: ")     + softAPIP);
  Serial.println((String) F("SoftAPIP mac address: ") + softAPmacAddress);
  
  WiFi.begin(WIFI_SID, WIFI_PW);
  for (int i = 0; i < 50 && WiFi.status() != WL_CONNECTED; i++) { // 30 Sec versuchen
    Serial.print(F("."));
    delay(500);
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    stationIP = WiFi.localIP().toString();
    Serial.println((String) F("Connected to ")        + wifiAP);
    Serial.println((String) F("Station IP address: ") + stationIP);
    Serial.println((String) F("AP1 SSID (RSSI): ")    + String(wifiAP + F(" (") + WifiGetRssiAsQuality(WiFi.RSSI()) + F("%)")));
    Serial.println(F("Connected"));
  } else { // switch to AP Mode only
    Serial.println(SOFT_AP_NAME);
    Serial.println(F("No connection to "));
    WiFi.disconnect();
    WiFi.mode(WIFI_AP);
    WiFi.begin(SOFT_AP_NAME, SOFT_AP_PW);
  }
  
  server.on(F("/"),              handleRoot);
  server.on(F("/Main.html"),     loadMain);
  server.on(F("/button"),        handlePushButton);
  server.onNotFound(handleWebRequests);
  
  server.begin(); 
  
  isWebServerActive = true;
  return true;
}

/** Handle the http requests. */
void MyWebServer::handleClient()
{
  if (isWebServerActive) {
    server.handleClient();
    dnsServer.processNextRequest();  
  }
}

/** Helper function to start a HTML table. */
void MyWebServer::AddTableBegin(String &info)
{
  info += F("<table style='width:100%'>");
}

/** Helper function to write one HTML row with no data. */
void MyWebServer::AddTableTr(String &info)
{
  info += F("<tr><th></th><td>&nbsp;</td></tr>");
}

/** Helper function to add one HTML table row line with data. */
void MyWebServer::AddTableTr(String &info, String name, String value)
{
  if (value != "") {
    info += F("<tr>");
    info += (String) F("<th>") + TextToXml(name)  + F("</th>");
    info += (String) F("<td>") + TextToXml(value) + F("</td>");
    info += F("</tr>");
  }
}
  
/** Helper function to add one HTML table end element. */
void MyWebServer::AddTableEnd(String &info)
{
  info += F("</table>");
}

/** Reads one string option from the url args. */
bool MyWebServer::GetOption(String id, String &option)
{
  if (server.hasArg(id)) {
    option = server.arg(id);
    return true;
  }
  return false;
}

/** Reads one long option from the URL args. */
bool MyWebServer::GetOption(String id, long &option)
{
  bool   ret = false;
  String opt = server.arg(id);
  
  if (opt != "") {
    if (opt.indexOf(F(":")) != -1) {
      ret = scanInterval(opt, option);
    }  else {
      option = atoi(opt.c_str());
      ret    = true;
    }
  }
  return ret;
}

/** Reads one double option from the URL args. */
bool MyWebServer::GetOption(String id, double &option)
{
  String opt = server.arg(id);
  
  if (opt != "") {
    option = atof(opt.c_str());
    return true;
  }
  return false;
}

/** Reads one bool option from the URL args. */
bool MyWebServer::GetOption(String id, bool &option)
{
  String opt = server.arg(id);
  
  option = false;
  if (opt != "") {
    if (opt == F("on")) {
      option = true;
    }
    return true;
  }
  return false;
}

/** Add a HTML br element. */
void MyWebServer::AddBr(String &info)
{
  info += F("<br />");
}

/** Helper function to load a file from the SPIFFS. */
bool MyWebServer::loadFromSpiffs(String path)
{
  bool ret = false;
  
  Serial.println("loadFromSpiffs: " + path);
  
  String dataType = F("text/plain");
  if(path.endsWith("/")) path += F("index.htm");
  
  if(path.endsWith(F(".src"))) path = path.substring(0, path.lastIndexOf("."));
  else if(path.endsWith(F(".html"))) dataType = F("text/html");
  else if(path.endsWith(F(".htm")))  dataType = F("text/html");
  else if(path.endsWith(F(".css")))  dataType = F("text/css");
  else if(path.endsWith(F(".js")))   dataType = F("application/javascript");
  else if(path.endsWith(F(".png")))  dataType = F("image/png");
  else if(path.endsWith(F(".gif")))  dataType = F("image/gif");
  else if(path.endsWith(F(".jpg")))  dataType = F("image/jpeg");
  else if(path.endsWith(F(".ico")))  dataType = F("image/x-icon");
  else if(path.endsWith(F(".xml")))  dataType = F("text/xml");
  else if(path.endsWith(F(".pdf")))  dataType = F("application/pdf");
  else if(path.endsWith(F(".zip")))  dataType = F("application/zip");
  
  File dataFile = SPIFFS.open(path.c_str(), "r");
  if (dataFile) {
    if (server.hasArg(F("download"))) {
      dataType = F("application/octet-stream");
    }
    if (server.streamFile(dataFile, dataType) == dataFile.size()) {
      ret = true;
    }
    dataFile.close();
  }
  return ret;
}

/** Redirect a root call to the Main.html site. */
void MyWebServer::handleRoot()
{
  Serial.println("handleRoot");
  
  server.sendHeader(F("Location"), F("Main.html"), true);
  server.send(302, F("text/plain"), "");
}

/** Sends the Main.html to the client. */
void MyWebServer::loadMain()
{
  Serial.println("loadMain");
  
  if (loadFromSpiffs(F("/Main.html"))) {
    return;
  }
  handleNotFound();
}

/** Handle the ajax call of the Main.html detail information. */
void MyWebServer::handleLoadMainInfo()
{
  String info;
  
  Serial.println("handleLoadMainInfo");
  AddTableBegin(info);
  if (WiFi.status() != WL_CONNECTED) {
    AddTableTr(info, F("AP SSID"), String(SOFT_AP_NAME));
  } else {
    AddTableTr(info, F("AP SSID (RSSI)"), String(wifiAP + F(" (") + WifiGetRssiAsQuality(WiFi.RSSI()) + F("%)")));
  }
  
  AddTableTr(info, F("Test Battery"), String(25.2, 2) + F(" V"));
  
  AddTableTr(info);
  AddTableEnd(info);
  
  server.send(200, F("text/html"), info);
}

/** Handle the pushButton Event */
void MyWebServer::handlePushButton()
{
  static bool enabled = true;

  String response = "Ok";
  Event  event;

  String buttonName;
  String buttonValue;
  
  Serial.println("handlePushButton");
  GetOption(F("name"),  buttonName);
  GetOption(F("value"), buttonValue);
  Serial.println("Name:  " + buttonName);
  Serial.println("Value: " + buttonValue);

       if (buttonName == "XLeft")  event.pushButton = Event::X_LEFT;
  else if (buttonName == "XRight") event.pushButton = Event::X_RIGHT;
  else if (buttonName == "YLeft")  event.pushButton = Event::Y_LEFT;
  else if (buttonName == "YRight") event.pushButton = Event::X_RIGHT;
  else if (buttonName == "ZLeft")  event.pushButton = Event::Z_LEFT;
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
  
  server.send(200, F("text/html"), response);
}

/** Handle if the url could not be found. */
void MyWebServer::handleNotFound()
{
  String message = F("File Not Found\n");
  
  Serial.println("handleNotFound");
  message += F("URI: ");
  message += server.uri();
  message += F("\nMethod: ");
  message += (server.method() == HTTP_GET) ? F("GET") : F("POST");
  message += F("\nArguments: ");
  message += server.args();
  message += F("\n");
  server.send(404, F("text/plain"), message);
}

/** Default for an unknown web request on not found. */
void MyWebServer::handleWebRequests()
{
  if (loadFromSpiffs(server.uri())) {
    return;
  }
  handleNotFound();
}
