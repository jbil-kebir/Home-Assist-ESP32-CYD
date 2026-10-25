#include "Diagnostic.h"

DiagCycle gDiag = {};
RTC_DATA_ATTR DiagCycle gDiagPrec = {};

// Exemple : "WiFi 2850 NTP 2900 MQTT 3000 (1 essai) Pub 3150 Echo 4200 Fin 4250 Dodo 4300"
String diagToString(const DiagCycle& d) {
  String s = "WiFi " + String(d.ulWifi) + " NTP " + String(d.ulNtp) + " MQTT " + String(d.ulMqtt);
  s += " (" + String(d.uiEssaisMqtt) + (d.uiEssaisMqtt > 1 ? " essais" : " essai");
  if (d.iDernierRc != 0) s += ", rc " + String(d.iDernierRc);
  s += ") Pub " + String(d.ulPub) + " Echo " + String(d.ulEcho) + " Fin " + String(d.ulFermeture);
  s += " Dodo " + String(d.ulSommeil);
  return s;
}
