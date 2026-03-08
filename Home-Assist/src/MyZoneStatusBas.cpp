
#include "global.h"
#include "MyZoneStatusBas.h"

//
// Retour
// -1 : Mesure invalide, on ne fait rien
//
int CZoneStatusBas::drawStatus(const String& msg, bool memorise/*=false*/) { 

  //Serial.printf("void CEcran::updateStatus()) - msg : %s - drawInterface : %d\n", msg.c_str(), drawInterface);
  String s = String(msg.c_str());
  if (memorise)
    setLastStatusMessage(s); // On enregistre le message
  mTft.fillRect(muiPosX, muiPosY, muiWidth, muiHight, muiBgColor);
  mTft.setTextColor(TFT_CYAN, TFT_BLACK);
  mTft.drawString(msg.c_str(), muiStatusX, muiStatusY, muiStatusFont);
  Serial.println(msg);

  return 0;

}

//
// Retour
// -1 : Mesure invalide, on ne fait rien
//
int CZoneStatusBas::drawStatus() { 

  //Serial.printf("void CEcran::updateStatus()) - msg : %s - drawInterface : %d\n", msg.c_str(), drawInterface);
  mTft.fillRect(muiPosX, muiPosY, muiWidth, muiHight, muiBgColor);
  mTft.setTextColor(TFT_CYAN, TFT_BLACK);
  mTft.drawString(mstLastStatusMessage.c_str(), muiStatusX, muiStatusY, muiStatusFont);
  Serial.println(mstLastStatusMessage);

  return 0;

}
String CZoneStatusBas::setLastStatusMessage(String& s) {
  String lastMsg = mstLastStatusMessage;
  mstLastStatusMessage = s;
  return lastMsg;
}

String CZoneStatusBas::getLastStatusMessage() {
  return mstLastStatusMessage;
}