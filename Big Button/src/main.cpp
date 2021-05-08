//Libraries
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPUtils.h>
#include <AudioManager.h>
#include <LEDManager.h>

//Constants
#define SERVER_TIME 700000;
#define BOUNCE_TIME 20
#define LED 13
#define BUTTON 33
#define TXD2 17 // connect to TX of MP3 Player module
#define RXD2 16 // connect to RX of MP3 Player module

#define audioKey  "aKey"
#define brightKey "bKey"
#define modeKey   "mKey"
#define volKey    "vKey"

//Parameters
String ssid = "BigButton_" + ESPUtils::getShortAddress();
const char *password = "sweetgum";

volatile bool notify;
volatile uint32_t bounceTime = 0;
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

bool serialBusy = false;
bool serverRunning = true;
unsigned long serverTime = millis() + SERVER_TIME;
AsyncWebServer server(80);

String restModes[] = {"None", "Solid", "Solid w Top", "Subtle", "Subtle w Top", "Cycle", "Cycle w Top"};
String audioTracks[] = {"Marc", "Joe", "Joe?", "Original"};

String getRadio(String section, int row, String label, int value)
{
  String fid = section + "_" + String(row);
  String checked = value == row ? "checked" : "";

  return "<input type='radio' id='" + fid + "' name='" + section + "' value='" + row + "' " + checked + "><label for='" + fid + "'> " + label + "</label><br>";
}

String getForm()
{
  String pageText = "<!DOCTYPE HTML><html><head><title>Salesforce Button</title>\
  <meta name='viewport' content='width=device-width, initial-scale=1'>\
  </head><body style='font-family: Arial;font-size:18px;'>\
  <h3 style='color:#17A0DB;'>Salesforce Button Settings</h3>\
  <form action='/get'><p>Idle Display:</p>";

  for (int i = 0; i < (sizeof(restModes) / sizeof(restModes[0])); i++)
  {
    pageText = pageText + getRadio(modeKey, i, restModes[i], LEDMan.patternIndex);
  }

  pageText = pageText + "<input type='range' id='bKey' name='bKey' min='5' max='255' value='"+ LEDMan.brightness +"'><label for='bKey'>Brightness</label><hr><p>Audio:</p>";

  for (int i = 0; i < (sizeof(audioTracks) / sizeof(audioTracks[0])); i++)
  {
    pageText = pageText + getRadio(audioKey, i, audioTracks[i], AudioMan.trackIndex);
  }

  return pageText + "<input type='range' id='vKey' name='vKey' min='1' max='30' value='"+ AudioMan.getVolume() +"'><label for='vKey'>Volume</label><br>\
  <input type='submit' value='Save' style='font-size:18px;'>\
  <a href='https://drive.google.com/drive/u/0/folders/11SwEiP9ogV3En5YXebt4sRtIhk40-01y?usp=sharing'>more info</a></form></body></html>";
}

void saveInput(AsyncWebServerRequest *request, String key)
{
  if (request->hasParam(key))
  {
    String value = request->getParam(key)->value();
    ESPUtils::setParameter(key, value);
  }
}

// Send web page with input fields to client
void handleConnect(AsyncWebServerRequest *request)
{
  request->send_P(200, "text/html", getForm().c_str());
}

void displaySettings(){
  Serial.println("audioKey: "+ ESPUtils::getParameterS(audioKey));
  Serial.println("volKey: "+ ESPUtils::getParameterS(volKey));
  Serial.println("modeKey: "+ ESPUtils::getParameterS(modeKey));
  Serial.println("brightKey: "+ ESPUtils::getParameterS(brightKey));
}

void loadSettings(){
  AudioMan.trackIndex = ESPUtils::getParameterS(audioKey,"0").toInt();
  AudioMan.setVolume(ESPUtils::getParameterS(volKey,"30").toInt());
  LEDMan.patternIndex = ESPUtils::getParameterS(modeKey,"2").toInt();
  LEDMan.brightness = ESPUtils::getParameterS(brightKey,"200").toInt();
  displaySettings();
}

void handleForm(AsyncWebServerRequest *request)
{
  serialBusy = true;
  saveInput(request, audioKey);
  saveInput(request, volKey);
  saveInput(request, modeKey);
  saveInput(request, brightKey);
  loadSettings();
  request->send_P(200, "text/html", getForm().c_str());
  serialBusy =  false;
}

void handleNotFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Not found");
}

void IRAM_ATTR handleInterrupt()
{
  if (bounceTime > xTaskGetTickCount() || notify)
  {
    return;
  }

  portENTER_CRITICAL_ISR(&mux);
  bounceTime = xTaskGetTickCount() + BOUNCE_TIME;
  notify = true;
  portEXIT_CRITICAL_ISR(&mux);
}

void setup()
{
  Serial.begin(115200);
  WiFi.mode(WIFI_AP_STA);
  Serial.println(ssid.c_str());
  WiFi.softAPConfig(IPAddress(10,10,10,1),IPAddress(10,10,10,0),IPAddress(255,255,255,0));
  WiFi.softAP(ssid.c_str());

  Serial.println();
  Serial.print("IP Address: ");

  IPAddress IP = WiFi.softAPIP();
  Serial.println(IP);
  server.on("/", handleConnect);
  server.on("/get", handleForm);
  server.onNotFound(handleNotFound);
  server.begin();

  AudioMan.begin(RXD2, TXD2);
  LEDMan.begin();

  loadSettings();
  pinMode(BUTTON, INPUT_PULLUP);
  attachInterrupt(BUTTON, handleInterrupt, RISING);
}

void loop()
{
  if(serialBusy){
    return;
  }

  if( serverRunning && serverTime < millis()){
    serverRunning = false;
    WiFi.disconnect(true);
  }

  if(notify){
    LEDMan.flashPattern();
    delay(5);
    AudioMan.playTrack();
    delay(15);
    portENTER_CRITICAL_ISR(&mux);
    notify = false;
    portEXIT_CRITICAL_ISR(&mux);
  }
  
  AudioMan.loop();
  LEDMan.loop();
}