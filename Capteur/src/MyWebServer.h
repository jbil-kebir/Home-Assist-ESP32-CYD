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
  CMqtt&  mqtt;
  CWifi *mWifi = nullptr;

  void handleRoot();
  void handleSave();
  void handleNotFound();

public:
  MyWebServer(CConfig& cfg, CMqtt& mq, CWifi *wifi) : server(80), config(cfg), mqtt(mq), mWifi(wifi) {}

  void setup();
  void loop();
};

#endif