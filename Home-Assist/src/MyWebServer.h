#ifndef MYWEBSERVER_H
#define MYWEBSERVER_H

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
  CEcran& ecran;
  CMqtt&  mqtt;
  CWifi *mWifi = nullptr;

  void handleForceOn();
  void handleForceOff();
  void handleToggleP();
  void handleToggleG();
  void handleChauffageSbOn();
  void handleChauffageSbOff();
  void handleRoot();
  void handleSave();
  void handleNotFound();

public:
  MyWebServer(CConfig& cfg, CEcran& ecr, CMqtt& mq, CWifi *wifi) : server(80), config(cfg), ecran(ecr), mqtt(mq), mWifi(wifi)/*, rcSwitch(rc)*/ {}

  void setup();
  void loop();
};

#endif