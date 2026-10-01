#include <Arduino.h>
#include "MyMax4466.h"
#ifdef CAPTEUR_MICRO_MAX4466

bool CMax4466::setup(const String pref) {
  mPrefixNVS = pref;
  mbMesureRemontee = false;

  // On charge les infos de config depuis le NVS
  loadFromNVS();

  analogReadResolution(12);                  // 0–4095
  analogSetPinAttenuation(mucPin, ADC_11db); // plage 0–3.3V
  pinMode(mucPin, INPUT);
  _initialized = true;

  // Mesure de la fréquence d'échantillonnage réelle (doit être proche de la consigne)
  unsigned long t0 = micros();
  acquireSamples();
  unsigned long dt = micros() - t0;
  Serial.printf("CMax4466::setup() - Bin FFT : %.1f Hz | Plage cible : %u–%u Hz | Échantillonnage réel : %.0f Hz (consigne %.0f Hz)\n",
                MAX4466_FREQ_ECHANTILLONNAGE / MAX4466_NB_ECHANTILLONS,
                muiFreqCible - muiFreqTol, muiFreqCible + muiFreqTol,
                MAX4466_NB_ECHANTILLONS * 1e6f / dt, MAX4466_FREQ_ECHANTILLONNAGE);

  return true;
}

//--------------------------------------------------------------------------
//  CMax4466::acquireSamples()
//
// Acquiert MAX4466_NB_ECHANTILLONS échantillons à MAX4466_FREQ_ECHANTILLONNAGE.
// Le MAX4466 sort un signal centré sur VCC/2 : on soustrait l'offset DC.
//--------------------------------------------------------------------------
void CMax4466::acquireSamples() {
  unsigned long us_period = (unsigned long)(1000000.0f / MAX4466_FREQ_ECHANTILLONNAGE);
  unsigned long t = micros();
  for (int i = 0; i < MAX4466_NB_ECHANTILLONS; i++) {
    vReal[i] = (float)analogRead(mucPin) - 2048.0f;   // centrage 12 bits
    vImag[i] = 0.0f;
    t += us_period;
    while ((long)(micros() - t) < 0) { /* busy wait */ }
  }
}

//--------------------------------------------------------------------------
//  CMax4466::getTargetAmplitude()
//
// Retourne l'amplitude max des bins FFT compris dans muiFreqCible ± muiFreqTol
//--------------------------------------------------------------------------
float CMax4466::getTargetAmplitude() {
  FFT.windowing(FFTWindow::Hann, FFTDirection::Forward);
  FFT.compute(FFTDirection::Forward);
  FFT.complexToMagnitude();

  float bin_width = MAX4466_FREQ_ECHANTILLONNAGE / MAX4466_NB_ECHANTILLONS;  // Hz/bin = 62.5 Hz
  int bin_lo = (int)(((float)muiFreqCible - (float)muiFreqTol) / bin_width);
  int bin_hi = (int)(((float)muiFreqCible + (float)muiFreqTol) / bin_width);
  bin_lo = max(1, bin_lo);
  bin_hi = min(MAX4466_NB_ECHANTILLONS / 2, bin_hi);

  float peak = 0.0f;
  for (int b = bin_lo; b <= bin_hi; b++) {
    if (vReal[b] > peak) peak = vReal[b];
  }
  return peak;
}

//--------------------------------------------------------------------------
//  CMax4466::loop()
//
// Chaque appel acquiert et analyse un bloc de 16 ms (bloquant).
//
// Retour
// 1 : Bip reconnu, état changé (publié)
// 0 : Bip reconnu, état inchangé (republié)
// -2 : Rien de nouveau
// -1 : Capteur non initialisé
// -9 : Capteur inactif
//--------------------------------------------------------------------------
int CMax4466::loop() {
  if (!active) return -9;
  if (!_initialized) return -1;
  int ret = -2;

  // Forçage périodique de la remontée (seulement si l'état est connu)
  if (miLastVal >= 0 && millis() - mulLastForcage > mulIntervalleForcageRemonteeMesure * 1000UL) {
    publie(true);
  }

  acquireSamples();
  float amp = getTargetAmplitude();
  bool tonDetecte = (amp >= (float)muiSeuil);
  unsigned long now = millis();

  if (mbTrace) {
    static unsigned long lastAmpMs = 0L;
    if (now - lastAmpMs >= 1000) {
      lastAmpMs = now;
      Serial.printf("CMax4466::loop() - amp=%.1f  ton=%d\n", amp, tonDetecte);
    }
  }

  if (tonDetecte && !mbBipActif) {
    // Front montant — début d'un bip
    mbBipActif = true;
    mulBipStart = now;
    Serial.printf("CMax4466::loop() - Bip début | amp=%.0f\n", amp);
  }
  else if (!tonDetecte && mbBipActif) {
    // Front descendant — fin du bip
    mbBipActif = false;
    unsigned long dur = now - mulBipStart;
    Serial.printf("CMax4466::loop() - Bip fin | durée=%lums\n", dur);

    if (dur >= muiCourtMin && dur <= muiCourtMax) {
      if (now - mulLastOnMs >= muiSilenceMin || mulLastOnMs == 0L) {
        mulLastOnMs = now;
        Serial.println("CMax4466::loop() - Bip COURT → ON");
        miNewVal = 1;
        ret = publie(false);
      }
      else Serial.println("CMax4466::loop() - Bip COURT ignoré (répétition RF)");
    }
    else if (dur >= muiLongMin && dur <= muiLongMax) {
      if (now - mulLastOffMs >= muiSilenceMin || mulLastOffMs == 0L) {
        mulLastOffMs = now;
        Serial.println("CMax4466::loop() - Bip LONG → OFF");
        miNewVal = 0;
        ret = publie(false);
      }
      else Serial.println("CMax4466::loop() - Bip LONG ignoré (répétition RF)");
    }
    else {
      Serial.printf("CMax4466::loop() - Durée hors plage (%lums), ignoré\n", dur);
    }
  }

  return ret;
}

int CMax4466::getLastMesure() const {
  return miLastVal;
}

//--------------------------------------------------------------------------
//  CMax4466::publie()
//
// Publie l'état courant. Chaque bip reconnu est publié, même si l'état
// est inchangé (resynchronise Home-Assist).
//
// Retour
// 1 : Etat changé
// 0 : Etat inchangé
// -1 : Etat inconnu (aucun bip reçu depuis l'installation)
//--------------------------------------------------------------------------
int CMax4466::publie(bool force/*=false*/) {
  int ret = 0;
  if (miNewVal < 0) return -1;
  if (miNewVal != miLastVal) {
    miLastVal = miNewVal;
    saveEtatToNVS(); // Pour republier l'état après un redémarrage
    ret = 1;
  }
  #ifdef _WIFI_MODE_
  #ifndef __DESACTIVE_ENVOI_MQTT__
  // Si échec de remontée MQTT, mbMesureRemontee sera à false
  mbMesureRemontee = publieSurMqtt(force);
  #endif
  #endif
  mulLastForcage = millis();
  return ret;
}

//
// L'argument force n'est utile que pour le debug. Ca permet de voir
// quand le message MQTT provient d'un forçage de mesure.
//
bool CMax4466::publieSurMqtt(bool force/*=false*/) {
    if (onMqttPublish == nullptr) return false;
    String sDate = (mDateTime != nullptr) ? mDateTime->getDate() : "DATE";
    String sTime = (mDateTime != nullptr) ? mDateTime->getTime() : "TIME";
    String sVal = nomEquipement + " OnOff " + sDate + " " + sTime + " " + String(miLastVal);
    if (force) sVal += " FORCE";
    Serial.println("CMax4466::publieSurMqtt() : " + sVal);
    return (onMqttPublish(mqttSubTopicState.c_str(), sVal.c_str()) == 0);
}

void CMax4466::setMqttPublishCallback(std::function<int(const char*, const char*)> cb) {
    onMqttPublish = cb;
}

void CMax4466::saveEtatToNVS() {
  prefs.begin(nvs_namespace, false);
  prefs.putChar((mPrefixNVS+"etat").c_str(), (int8_t)miLastVal);
  prefs.end();
}

void CMax4466::loadFromNVS() {
  prefs.begin(nvs_namespace, true);

  nomEquipement = prefs.getString((mPrefixNVS+"nom").c_str(), "Noeud");
  mucPin = prefs.getUShort((mPrefixNVS+"pin").c_str(), CAPTEUR_MICRO_MAX4466_PIN);
  mqttSubTopic = prefs.getString((mPrefixNVS+"subtopic").c_str(), "noeud/");
  active = prefs.getBool((mPrefixNVS+"active").c_str(), false);
  mulIntervalleForcageRemonteeMesure = prefs.getLong((mPrefixNVS+"force").c_str(), mulDefaultIntervalleForcageRemonteeMesure);
  muiFreqCible = prefs.getUShort((mPrefixNVS+"freq").c_str(), MAX4466_DEFAULT_FREQ_CIBLE);
  muiFreqTol = prefs.getUShort((mPrefixNVS+"tol").c_str(), MAX4466_DEFAULT_FREQ_TOL);
  muiSeuil = prefs.getUShort((mPrefixNVS+"seuil").c_str(), MAX4466_DEFAULT_SEUIL);
  muiCourtMin = prefs.getUShort((mPrefixNVS+"cmin").c_str(), MAX4466_DEFAULT_COURT_MIN);
  muiCourtMax = prefs.getUShort((mPrefixNVS+"cmax").c_str(), MAX4466_DEFAULT_COURT_MAX);
  muiLongMin = prefs.getUShort((mPrefixNVS+"lmin").c_str(), MAX4466_DEFAULT_LONG_MIN);
  muiLongMax = prefs.getUShort((mPrefixNVS+"lmax").c_str(), MAX4466_DEFAULT_LONG_MAX);
  muiSilenceMin = prefs.getUShort((mPrefixNVS+"silence").c_str(), MAX4466_DEFAULT_SILENCE_MIN);
  mbTrace = prefs.getBool((mPrefixNVS+"trace").c_str(), false);
  miLastVal = prefs.getChar((mPrefixNVS+"etat").c_str(), -1);
  miNewVal = miLastVal;

  // On forme les subtopic MQTT
  domotique_prefix = default_domotique_topic_prefix;

  mqttSubTopicCommand = domotique_prefix + mqttSubTopic + "command";
  mqttSubTopicState   = domotique_prefix + mqttSubTopic + "state";

  prefs.end();
}

void CMax4466::saveToNVS() {
  prefs.begin(nvs_namespace, false);

  prefs.putString((mPrefixNVS+"nom").c_str(), nomEquipement);
  prefs.putUShort((mPrefixNVS+"pin").c_str(), mucPin);
  prefs.putString((mPrefixNVS+"subtopic").c_str(), mqttSubTopic);
  prefs.putBool((mPrefixNVS+"active").c_str(), active);
  prefs.putLong((mPrefixNVS+"force").c_str(), mulIntervalleForcageRemonteeMesure);
  prefs.putUShort((mPrefixNVS+"freq").c_str(), muiFreqCible);
  prefs.putUShort((mPrefixNVS+"tol").c_str(), muiFreqTol);
  prefs.putUShort((mPrefixNVS+"seuil").c_str(), muiSeuil);
  prefs.putUShort((mPrefixNVS+"cmin").c_str(), muiCourtMin);
  prefs.putUShort((mPrefixNVS+"cmax").c_str(), muiCourtMax);
  prefs.putUShort((mPrefixNVS+"lmin").c_str(), muiLongMin);
  prefs.putUShort((mPrefixNVS+"lmax").c_str(), muiLongMax);
  prefs.putUShort((mPrefixNVS+"silence").c_str(), muiSilenceMin);
  prefs.putBool((mPrefixNVS+"trace").c_str(), mbTrace);

  prefs.end();
}

void CMax4466::setActive(bool state) {
  active = state;
  prefs.begin(nvs_namespace, false);
  prefs.putBool((mPrefixNVS+"active").c_str(), state);
  prefs.end();
}

void CMax4466::loadFromWebServer (WebServer& server) {
  if (server.hasArg((mPrefixNVS+"nom").c_str())) nomEquipement = server.arg((mPrefixNVS+"nom").c_str());
  if (server.hasArg((mPrefixNVS+"pin").c_str())) mucPin = server.arg((mPrefixNVS+"pin")).toInt();
  if (server.hasArg((mPrefixNVS+"active").c_str())) active = true; else active = false;
  if (server.hasArg((mPrefixNVS+"trace").c_str())) mbTrace = true; else mbTrace = false;
  if (server.hasArg((mPrefixNVS+"subtopic").c_str())) mqttSubTopic = server.arg((mPrefixNVS+"subtopic").c_str());
  if (server.hasArg((mPrefixNVS+"force").c_str())) mulIntervalleForcageRemonteeMesure = server.arg((mPrefixNVS+"force")).toInt();
  if (server.hasArg((mPrefixNVS+"freq").c_str())) muiFreqCible = server.arg((mPrefixNVS+"freq")).toInt();
  if (server.hasArg((mPrefixNVS+"tol").c_str())) muiFreqTol = server.arg((mPrefixNVS+"tol")).toInt();
  if (server.hasArg((mPrefixNVS+"seuil").c_str())) muiSeuil = server.arg((mPrefixNVS+"seuil")).toInt();
  if (server.hasArg((mPrefixNVS+"cmin").c_str())) muiCourtMin = server.arg((mPrefixNVS+"cmin")).toInt();
  if (server.hasArg((mPrefixNVS+"cmax").c_str())) muiCourtMax = server.arg((mPrefixNVS+"cmax")).toInt();
  if (server.hasArg((mPrefixNVS+"lmin").c_str())) muiLongMin = server.arg((mPrefixNVS+"lmin")).toInt();
  if (server.hasArg((mPrefixNVS+"lmax").c_str())) muiLongMax = server.arg((mPrefixNVS+"lmax")).toInt();
  if (server.hasArg((mPrefixNVS+"silence").c_str())) muiSilenceMin = server.arg((mPrefixNVS+"silence")).toInt();
}

String CMax4466::getHTML() {
  String html = "";
  html =  "<h2>Configuration de " + nomEquipement + " (micro MAX4466)</h2>"
      "<div class=\"row\">"
        "<div><label>Nom</label><input type=\"text\" name=" + (mPrefixNVS+"nom") + " value=\"" + nomEquipement + "\"></div>"
        "<div class=\"checkbox-row\"><label>Actif</label><input type=\"checkbox\" name=" + (mPrefixNVS+"active") + " value=\"1\"" + String(active ? " checked" : "") + "></div>"
      "</div>"
      "<div class=\"row\">"
        "<div><label>Pin</label><input type=\"text\" name=" + (mPrefixNVS+"pin") + " value=\"" + mucPin + "\"></div>"
        "<div class=\"checkbox-row\"><label>Trace amplitude</label><input type=\"checkbox\" name=" + (mPrefixNVS+"trace") + " value=\"1\"" + String(mbTrace ? " checked" : "") + "></div>"
      "</div>"
      "<div class=\"row\">"
        "<div><label>Fréquence cible (Hz)</label><input type=\"text\" name=" + (mPrefixNVS+"freq") + " value=\"" + muiFreqCible + "\"></div>"
        "<div><label>Tolérance (± Hz)</label><input type=\"text\" name=" + (mPrefixNVS+"tol") + " value=\"" + muiFreqTol + "\"></div>"
        "<div><label>Seuil amplitude</label><input type=\"text\" name=" + (mPrefixNVS+"seuil") + " value=\"" + muiSeuil + "\"></div>"
      "</div>";
  html += "<div class=\"row\">"
        "<div><label>Bip court ON min (ms)</label><input type=\"text\" name=" + (mPrefixNVS+"cmin") + " value=\"" + muiCourtMin + "\"></div>"
        "<div><label>Bip court ON max (ms)</label><input type=\"text\" name=" + (mPrefixNVS+"cmax") + " value=\"" + muiCourtMax + "\"></div>"
      "</div>"
      "<div class=\"row\">"
        "<div><label>Bip long OFF min (ms)</label><input type=\"text\" name=" + (mPrefixNVS+"lmin") + " value=\"" + muiLongMin + "\"></div>"
        "<div><label>Bip long OFF max (ms)</label><input type=\"text\" name=" + (mPrefixNVS+"lmax") + " value=\"" + muiLongMax + "\"></div>"
      "</div>"
      "<div class=\"row\">"
        "<div><label>Silence min entre bips (ms)</label><input type=\"text\" name=" + (mPrefixNVS+"silence") + " value=\"" + muiSilenceMin + "\"></div>"
        "<div><label>Intervalle forçage remontée (s)</label><input type=\"text\" name=" + (mPrefixNVS+"force") + " value=\"" + mulIntervalleForcageRemonteeMesure + "\"></div>"
      "</div>";
  html += "<div class=\"row\">"
        "<div><label>Topic prefix MQTT</label><input type=\"text\" name=" + (mPrefixNVS+"subtopic") + " value=\"" + mqttSubTopic + "\"></div>"
      "</div>";

  return html;
}

void CMax4466::print() const {
  Serial.printf("     Nom                  : %s\n", nomEquipement.c_str());
  Serial.printf("     Actif                : %s\n", active ? "OUI" : "NON");
  Serial.printf("     Pin                  : %d\n", mucPin);
  Serial.printf("     Fréquence cible      : %u ± %u Hz\n", muiFreqCible, muiFreqTol);
  Serial.printf("     Seuil amplitude      : %u\n", muiSeuil);
  Serial.printf("     Bip court (ON)       : %u - %u ms\n", muiCourtMin, muiCourtMax);
  Serial.printf("     Bip long (OFF)       : %u - %u ms\n", muiLongMin, muiLongMax);
  Serial.printf("     Silence min          : %u ms\n", muiSilenceMin);
  Serial.printf("     Forcage remontée     : %ld s\n", mulIntervalleForcageRemonteeMesure);
  Serial.printf("     Trace amplitude      : %s\n", mbTrace ? "OUI" : "NON");
  Serial.printf("     Dernier état         : %d\n", miLastVal);
  Serial.printf("     MQTTSubTopic         : %s\n", mqttSubTopic.c_str());
  Serial.printf("     MQTTCmd              : %s\n", mqttSubTopicCommand.c_str());
  Serial.printf("     MQTTState            : %s\n", mqttSubTopicState.c_str());
}

//
// Retour
// 0 : RAS
// -1 : Mauvais nom d'équipement
// -2 : Commande incomplète
//
int CMax4466::handleMqttCommand(const String& payload) {
  int ret = 0;
  String cmd = payload;
  cmd.toUpperCase();
  cmd.trim();

  MQTT_COMMAND_5 st;
  int idx = 0;
  Serial.println("=== CMax4466::handleMqttCommand ===");
  Serial.println("cmd : " + cmd);
  int sp = cmd.indexOf(' ', idx);
  if (sp < 0) {
    Serial.println("Commande incomplète : " + cmd);
    return -2; // Commande incomplète
  }
  st.sExpediteur = cmd.substring(0, sp);
  String s = nomEquipement; s.toUpperCase();
  if (st.sExpediteur != s) {
    Serial.println("Mauvais nom d'équipement (" + nomEquipement + ") : " + st.sExpediteur);
    return -1; // Mauvais nom d'équipement
  }
  idx = sp + 1;
  sp = cmd.indexOf(' ', idx);
  st.sCommand = (sp < 0) ? cmd.substring(idx) : cmd.substring(idx, sp);
  if (st.sCommand == "MESURE") { // Demande de remontée de l'état
    ret = publie(true);
  }
  return ret;
}

#endif // CAPTEUR_MICRO_MAX4466
