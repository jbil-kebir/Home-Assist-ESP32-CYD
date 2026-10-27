#include "global.h" // Serial -> journal web (voir MyJournal.h)
//#include <Arduino.h>
#include "MyDateTime.h"
#include <esp_sntp.h>
#include <esp_system.h>

// Heure fiable = synchronisée par NTP. Le drapeau est conservé pendant un deep sleep (mémoire RTC) et remis à faux
// à tout autre redémarrage : la RTC peut alors contenir une heure aberrante (vu : 2037 après un téléversement)
// que getLocalTime() croirait valide (il ne teste que l'année > 2016). Sans NTP (LoRa seul) : jetons DATE / TIME.
static RTC_DATA_ATTR bool gbHeureSynchro = false;

static void onSynchroNtp(struct timeval*) { gbHeureSynchro = true; }

static bool heureFiable() {
  static bool bInit = false;
  if (!bInit) {
    bInit = true;
    if (esp_reset_reason() != ESP_RST_DEEPSLEEP) gbHeureSynchro = false;
  }
  return gbHeureSynchro;
}

bool CMyDateTime::isTimeValid() {
  struct tm timeinfo;
  return heureFiable() && getLocalTime(&timeinfo, 0);
}

// Fonction pour obtenir la date et l'heure en String
String CMyDateTime::getDateTime() {
  struct tm timeinfo;
  if (!heureFiable() || !getLocalTime(&timeinfo, 0)) { // 0 : sans attente (défaut 5 s, ×2 par message en LoRa seul, heure jamais synchronisée)
    return "Erreur: Temps non disponible";
  }
  
  char buffer[64];
  strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M:%S", &timeinfo);
  return String(buffer);
}

// Autres formats possibles
// Sans heure valide (LoRa seul) : jetons DATE / TIME, remplacés par la date et l'heure de Relais-Lora
String CMyDateTime::getDate() {
  struct tm timeinfo;
  if (!heureFiable() || !getLocalTime(&timeinfo, 0)) {
    return "DATE";
  }
  
  char buffer[32];
  strftime(buffer, sizeof(buffer), "%d/%m/%Y", &timeinfo);
  return String(buffer);
}

String CMyDateTime::getTime() {
  struct tm timeinfo;
  if (!heureFiable() || !getLocalTime(&timeinfo, 0)) {
    return "TIME";
  }
  
  char buffer[32];
  strftime(buffer, sizeof(buffer), "%H:%M:%S", &timeinfo);
  return String(buffer);
}

// Format ISO 8601
String CMyDateTime::getDateTimeISO() {
  struct tm timeinfo;
  if (!heureFiable() || !getLocalTime(&timeinfo, 0)) {
    return "Erreur";
  }
  
  char buffer[64];
  strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", &timeinfo);
  return String(buffer);
}

int CMyDateTime::setup() {
    int ret = 0;
    sntp_set_time_sync_notification_cb(onSynchroNtp);
    configTzTime(tzFrance, ntpServer);
    // Au réveil de deep sleep, l'heure synchronisée avant le sommeil est gardée : on rend la main aussitôt
    // (la synchro NTP se fait en tâche de fond). Sinon, on attend la 1re synchro (NTP_TIMEOUT_MS au plus).
    unsigned long ulDebut = millis();
    while (!heureFiable() && millis() - ulDebut < NTP_TIMEOUT_MS) delay(20);
    if (!heureFiable()) {
      Serial.println("Heure non synchronisée");
      ret = -1;
    }

    return ret;
}