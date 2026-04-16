#ifndef MYWEBSERVER_H
#define MYWEBSERVER_H

#include "global.h"
#include <WiFi.h>
#include <WebServer.h>
#include "MyWifi.h"  // Ta classe config
#include "MyMqtt.h"  // Ta classe config
#include "MyConfig.h"  // Ta classe config

class CMyRCSwitch;
class CWifi;

class MyWebServer {
private:
  WebServer server;
  CConfig& config;
  #ifndef __DESACTIVE_ENVOI_MQTT__
  CMqtt&  mqtt;
  #endif
  CWifi *mWifi = nullptr;

  void handleRoot();
  void handleSave();
  void handleNotFound();

public:
  #ifndef __DESACTIVE_ENVOI_MQTT__
  MyWebServer(CConfig& cfg, CMqtt& mq, CWifi *wifi) : server(80), config(cfg), mqtt(mq), mWifi(wifi) {}
  #else
  MyWebServer(CConfig& cfg, CWifi *wifi) : server(80), config(cfg), mWifi(wifi) {}
  #endif
  
  void setup();
  void loop();
};

#endif