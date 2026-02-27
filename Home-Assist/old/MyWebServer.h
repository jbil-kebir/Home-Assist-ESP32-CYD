#ifndef MYWEBSERVER_H
#define MYWEBSERVER_H

#include <WiFi.h>
#include <WebServer.h>
#include "MyConfig.h"  // Ta classe config

class MyWebServer {
private:
  WebServer server;
  CConfig& config;
  CRadioTX& radioTX;
  CEcran& ecran;
  CMqtt&  mqtt;

  void handleForceOn();
  void handleForceOff();
  void handleRoot();
  void handleSave();
  void handleNotFound();

public:
  MyWebServer(CConfig& cfg, CRadioTX& tx, CEcran& ecr, CMqtt& mq) : server(80), config(cfg), radioTX(tx), ecran(ecr), mqtt(mq) {}

  void setup();
  void loop();
};

#endif