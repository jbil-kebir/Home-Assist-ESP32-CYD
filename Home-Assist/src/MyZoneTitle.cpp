
#include "global.h"
#include "MyConfig.h"
#include "MyZoneTitle.h"


//
// Retour
// 0 : RAS
// -1 : Pb de pointeur
//
int CZoneTitle::drawTitle(const String& ip) { 
  if (mConfig == nullptr) return -1;

  mTft.fillRect(muiPosX, muiPosY, muiWidth, muiHight, TITRE_C);

  mTft.setTextColor(TITRE_MSG_C);

  String title = mConfig->nomEquipement + String(" V") + String(VERSION);// + " - " + String(mConfig.wifiInfo->nomEquipement);
  /*if (mConfig.mWifi != nullptr) {
    title += " - " + String(mConfig.mWifi->nomEquipement);
  }*/
  if (ip.length() > 0) {
    title += " - " + ip;
  } else {
    title += " - ...";
  }

  mTft.drawCentreString(title.c_str(), TITRE_MSG_POS_X, TITRE_MSG_POS_Y, TITRE_MSG_FONT);
  return 0;

}

//
// Retour
// 0 : RAS
// -1 : Pb de pointeur
//
int CZoneTitle::drawTitle() { 

  String ip = WiFi.localIP().toString();

  int ret = drawTitle(ip);

  return ret;

}