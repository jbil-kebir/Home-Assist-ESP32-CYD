#include "global.h" // Serial -> journal web (voir MyJournal.h)
//#include <Arduino.h>
#include "MyDateTime.h"

// Fonction pour obtenir la date et l'heure en String
String CMyDateTime::getDateTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 0)) { // 0 : sans attente (défaut 5 s, ×2 par message en LoRa seul, heure jamais synchronisée)
    return "Erreur: Temps non disponible";
  }
  
  char buffer[64];
  strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M:%S", &timeinfo);
  return String(buffer);
}

// Autres formats possibles
String CMyDateTime::getDate() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 0)) {
    return "Erreur";
  }
  
  char buffer[32];
  strftime(buffer, sizeof(buffer), "%d/%m/%Y", &timeinfo);
  return String(buffer);
}

String CMyDateTime::getTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 0)) {
    return "Erreur";
  }
  
  char buffer[32];
  strftime(buffer, sizeof(buffer), "%H:%M:%S", &timeinfo);
  return String(buffer);
}

// Format ISO 8601
String CMyDateTime::getDateTimeISO() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 0)) {
    return "Erreur";
  }
  
  char buffer[64];
  strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", &timeinfo);
  return String(buffer);
}

int CMyDateTime::setup() {
    int ret = 0;
    configTzTime(tzFrance, ntpServer);
    // Au réveil de deep sleep, l'horloge RTC a gardé l'heure : getLocalTime() rend la main aussitôt
    // (la synchro NTP se fait en tâche de fond). À la mise sous tension, on attend la 1re synchro.
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, NTP_TIMEOUT_MS)) {
      Serial.println("Heure non synchronisée");
      ret = -1;
    }

    return ret;
}