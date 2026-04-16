#include "global.h"
#ifdef __LOCAL_MODE__
#include "MyRCSwitch.h"
#include <ELECHOUSE_CC1101_SRC_DRV.h>
#include "MyConfig.h"

void CMyRCSwitch::setup() {
    mySwitch = RCSwitch();
    mySwitch.enableReceive(CC1101_GDO0);

}
void CMyRCSwitch::afficheDetailRCS() {
    DBG(DBG_ACTIONNEURS, "CMyRCSwitch::loop() - Reçu : ");
    DBGLN(DBG_ACTIONNEURS, mySwitch.getReceivedValue());
    DBG(DBG_ACTIONNEURS, " / ");
    DBGLN(DBG_ACTIONNEURS, mySwitch.getReceivedBitlength());
    DBG(DBG_ACTIONNEURS, " bits / Protocol: ");
    DBGLN(DBG_ACTIONNEURS, mySwitch.getReceivedProtocol());
    DBG(DBG_ACTIONNEURS, " / RSSI approx: ");
    DBGLN(DBG_ACTIONNEURS, ELECHOUSE_cc1101.getRssi());  // si tu as la lib chargée

}

unsigned char CMyRCSwitch::getIDFromRSCCode(unsigned long code) {
  unsigned char id = ID_NONE;
  id = (code >> 16) & 0xFF;
  return id;
}

//
// Retour
// 0 : RAS
// -2 : Mauvais protocole
// -3 : Mauvaise taille de bits (!= 24)
// -9 : Equipement inactif
// -10 : ID équipement inconnu
// 
int CMyRCSwitch::loop() {
  int ret = 0;

  if (mySwitch.available()) {
    if (mySwitch.getReceivedBitlength() != 24) return -3;
    //if (mySwitch.getReceivedProtocol() != protocol) return -2;

    unsigned long code = mySwitch.getReceivedValue();
    
    DBG(DBG_ACTIONNEURS, "int CMyRCSwitch::loop() - Code : 0x%lx\n", code);

    unsigned char id = getIDFromRSCCode(code);
    switch (id) {
      case 1: // ThCave
        ret = config->mRemoteThCave->handleRCSwitchCode(code);
      break;
      case 2: // ThRemise
        ret = config->mRemoteThRemise->handleRCSwitchCode(code);
      break;
      default:
        DBG(DBG_ACTIONNEURS, "Code : 0x%lx - Identifiant %d inconnu\n", code, id);
        ret = -10;
      break;
    }

    // On se prépare à la prochaine réception 
    mySwitch.resetAvailable();
  }

  return ret;
}

int CMyRCSwitch::toggleDevice() {
  int ret = 0;
    #ifdef DISABLE_EFFECTEUR
    DBG(DBG_ACTIONNEURS, "void CMyRCSwitch::toggleDevice() - Effecteur désactivé.\n");
    return ret;
    #endif

  mySwitch.disableReceive(); 
  ELECHOUSE_cc1101.setMHZ(frequency);
  ELECHOUSE_cc1101.setModulation(2);
  ELECHOUSE_cc1101.setDeviation(0);
  ELECHOUSE_cc1101.setPktFormat(3);
  ELECHOUSE_cc1101.setSyncMode(0);
  ELECHOUSE_cc1101.setCrc(0);
  ELECHOUSE_cc1101.setPA(12);
  ELECHOUSE_cc1101.SetTx();
  delay(10);

  mySwitch.enableTransmit(CC1101_GDO0);

  mySwitch.setProtocol(protocol);
  mySwitch.setPulseLength(pulseLength);
  mySwitch.setRepeatTransmit(repeat);
  String s = String(frequency) + " MHz, " + String(code, HEX) + ", " + String(protocol) + ", " + String(pulseLength) + ", " + String(repeat);
  DBG(DBG_ACTIONNEURS, "CMyRCSwitch::toggleDevice() - Envoi %s\n", s.c_str());
  mySwitch.send(code, 24);

  mySwitch.disableTransmit();
// Test Grok
/*ELECHOUSE_cc1101.setMHZ(433.92);
ELECHOUSE_cc1101.setModulation(2);     
ELECHOUSE_cc1101.setDeviation(47.607); // Deviation standard pour bonne portée
ELECHOUSE_cc1101.setDRate(4.8);        // Débit bas = meilleure sensibilité
ELECHOUSE_cc1101.setPA(12);*/            // Max puissance
  pinMode(CC1101_GDO0, INPUT);
  ELECHOUSE_cc1101.SetRx();
  mySwitch.enableReceive(CC1101_GDO0);

  return ret;
}
#endif // __LOCAL_MODE__