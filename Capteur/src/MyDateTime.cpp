//#include <Arduino.h>
#include "MyDateTime.h"

// Fonction pour obtenir la date et l'heure en String
String CMyDateTime::getDateTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "Erreur: Temps non disponible";
  }
  
  char buffer[64];
  strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M:%S", &timeinfo);
  return String(buffer);
}

// Autres formats possibles
String CMyDateTime::getDate() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "Erreur";
  }
  
  char buffer[32];
  strftime(buffer, sizeof(buffer), "%d/%m/%Y", &timeinfo);
  return String(buffer);
}

String CMyDateTime::getTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "Erreur";
  }
  
  char buffer[32];
  strftime(buffer, sizeof(buffer), "%H:%M:%S", &timeinfo);
  return String(buffer);
}

// Format ISO 8601
String CMyDateTime::getDateTimeISO() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "Erreur";
  }
  
  char buffer[64];
  strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", &timeinfo);
  return String(buffer);
}

int CMyDateTime::setup() {
    int ret = 0;
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    // Attendre la synchronisation
    delay(4000);

    return ret;
}