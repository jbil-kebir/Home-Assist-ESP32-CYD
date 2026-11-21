#include "MyOta.h"
#include "global.h"
#include <ArduinoOTA.h>
#include <Update.h>
#include <Preferences.h>
#include <esp_ota_ops.h>
#include <esp_timer.h>
#include <esp_task_wdt.h>
#include <vector>

COta gOta;

// Étiquette du firmware, cherchée par la page /update de l'appareil qui le reçoit : refuse le firmware d'un autre
// appareil (ETIQUETTE_FIRMWARE, global.h). Référencée dans begin() pour que l'éditeur de liens la garde.
extern "C" __attribute__((used)) const char gEtiquetteFirmware[] = "@@PILOTAGE_FW:" ETIQUETTE_FIRMWARE "@@";

// Validation différée (Arduino 3.x, esp32-hal-misc.c) : sans elle, initArduino() validerait le nouveau firmware dès
// son démarrage, et le retour arrière ne servirait jamais. Validé par COta::confirme(), une fois MQTT connecté.
extern "C" bool verifyRollbackLater() { return true; }

static esp_timer_handle_t sMinuterie = nullptr;

static void retourArriere(void*) {
  // Pas de connexion MQTT à temps : l'ancien firmware reprend la main (il publiera RETOUR à sa connexion)
  Serial.println("[OTA] Nouveau firmware non validé à temps : retour au précédent");
  esp_ota_mark_app_invalid_rollback_and_reboot();
}

static bool aLEssai() {
  esp_ota_img_states_t etat;
  return esp_ota_get_state_partition(esp_ota_get_running_partition(), &etat) == ESP_OK
         && etat == ESP_OTA_IMG_PENDING_VERIFY;
}

void COta::debutDemarrage() {
  if (!aLEssai()) return;
  Serial.printf("[OTA] Nouveau firmware à l'essai : validation attendue dans %d s\n", OTA_DELAI_VALIDATION_S);
  esp_timer_create_args_t args = {};
  args.callback = &retourArriere;
  args.name = "ota_retour";
  if (esp_timer_create(&args, &sMinuterie) == ESP_OK)
    esp_timer_start_once(sMinuterie, (uint64_t)OTA_DELAI_VALIDATION_S * 1000000ULL);
}

void COta::begin(const String& nomTopic, const char* motDePasse, const String& version,
                 std::function<void(const char*, const char*)> publie, std::function<String()> horodatage,
                 bool demarrerArduinoOta) {
  msNomTopic = nomTopic;
  msMotDePasse = motDePasse;
  msVersion = version;
  mPublie = publie;
  mHorodatage = horodatage;
  Serial.printf("[OTA] Prêt (%s)\n", gEtiquetteFirmware);

  String sHote = nomTopic; sHote.toLowerCase();
  ArduinoOTA.setHostname(sHote.c_str());
  ArduinoOTA.setPassword(motDePasse);
  ArduinoOTA.setRebootOnSuccess(true);
  ArduinoOTA.onStart([this]() { this->publie("DEBUT", "(PlatformIO)"); });
  ArduinoOTA.onEnd([this]() {
    noteMiseAJour("PlatformIO");
    this->publie("TRANSFERT", ": terminé, redémarrage");
    delay(300);   // Laisse partir le message MQTT avant le redémarrage
  });
  ArduinoOTA.onError([this](ota_error_t e) {
    const char* r = (e == OTA_AUTH_ERROR) ? "mot de passe refusé" : (e == OTA_BEGIN_ERROR) ? "début refusé (place ?)"
                  : (e == OTA_CONNECT_ERROR) ? "connexion perdue" : (e == OTA_RECEIVE_ERROR) ? "réception interrompue"
                  : (e == OTA_END_ERROR) ? "fin refusée (fichier incomplet ou invalide)" : "erreur inconnue";
    this->publie("ECHEC", String(": ") + r + " (PlatformIO)");
  });
  // Transfert d'un seul tenant dans ArduinoOTA.handle() : le chien de garde de loop() (Capteur) redémarrerait le nœud
  ArduinoOTA.onProgress([](unsigned int, unsigned int) { esp_task_wdt_reset(); });
  if (demarrerArduinoOta) demarreArduinoOta();
}

static bool sbArduinoOtaDemarre = false;

void COta::demarreArduinoOta() {
  if (sbArduinoOtaDemarre) return;
  ArduinoOTA.begin();
  sbArduinoOtaDemarre = true;
}

void COta::loop() {
  if (sbArduinoOtaDemarre) ArduinoOTA.handle();
}

void COta::publie(const String& etape, const String& detail) {
  String sMsg = etape + " " + msVersion + " " + (mHorodatage ? mHorodatage() : String("")) +
                (detail.length() ? " " + detail : String(""));
  Serial.println("[OTA] " + sMsg);
  if (mPublie) mPublie((String(OTA_TOPIC_RACINE) + msNomTopic).c_str(), sMsg.c_str());
}

// Transfert réussi, juste avant le redémarrage : au démarrage suivant, confirme() saura si le nouveau firmware a pris
// la main (OK) ou si l'ancien est revenu (RETOUR)
void COta::noteMiseAJour(const char* source) {
  const esp_partition_t* cible = esp_ota_get_next_update_partition(nullptr);
  Preferences prefs;
  prefs.begin("ota", false);
  prefs.putBool("attente", true);
  prefs.putString("cible", cible ? cible->label : "");
  prefs.putString("source", source);
  prefs.putString("avant", msVersion);
  prefs.end();
}

void COta::confirme() {
  if (mbConfirme) return;
  mbConfirme = true;
  if (aLEssai()) {
    if (sMinuterie) esp_timer_stop(sMinuterie);
    esp_ota_mark_app_valid_cancel_rollback();
    Serial.println("[OTA] Nouveau firmware validé");
  }
  Preferences prefs;
  prefs.begin("ota", false);
  if (prefs.getBool("attente", false)) {
    String sCible = prefs.getString("cible", "");
    String sSource = prefs.getString("source", "");
    String sAvant = prefs.getString("avant", "");
    const esp_partition_t* actif = esp_ota_get_running_partition();
    if (actif && sCible == actif->label)
      publie("OK", "(" + sSource + ", version précédente " + sAvant + ")");
    else
      publie("RETOUR", ": le nouveau firmware a planté au démarrage ou ne s'est pas connecté à MQTT en " +
                       String(OTA_DELAI_VALIDATION_S) + " s (" + sSource + ")");
    prefs.putBool("attente", false);
  }
  prefs.end();
}

// Étiquette sans le suffixe « _ota » : un profil PlatformIO <nœud>_ota (téléversement réseau) produit le même firmware
// que le profil USB <nœud> ; les deux étiquettes désignent le même appareil
static String sansSuffixeOta(const String& s) {
  return s.endsWith("_ota") ? s.substring(0, s.length() - 4) : s;
}

// Cherche « @@PILOTAGE_FW:<appareil>@@ » dans le flux reçu. Préfixe assemblé à l'exécution : écrit en entier dans le
// code, il figurerait une 2e fois dans le firmware (sans nom d'appareil derrière : ignoré de toute façon)
void COta::chercheEtiquette(const uint8_t* data, size_t len) {
  static const size_t GARDE = 64;
  String sPrefixe = String("@@PILOT") + "AGE_FW:";
  std::vector<uint8_t> buf(msFinPrecedente.begin(), msFinPrecedente.end());
  buf.insert(buf.end(), data, data + len);
  const size_t lp = sPrefixe.length();
  for (size_t i = 0; i + lp < buf.size(); i++) {
    if (memcmp(&buf[i], sPrefixe.c_str(), lp) != 0) continue;
    String sNom;
    for (size_t j = i + lp; j + 1 < buf.size() && j < i + lp + 40; j++) {
      if (buf[j] == '@' && buf[j + 1] == '@') {
        if (sansSuffixeOta(sNom) == sansSuffixeOta(ETIQUETTE_FIRMWARE)) mbEtiquetteOk = true;
        else if (sNom.length()) msEtiquetteTrouvee = sNom;
        break;
      }
      if (buf[j] < 32 || buf[j] > 126) break;
      sNom += (char)buf[j];
    }
  }
  size_t debut = buf.size() > GARDE ? buf.size() - GARDE : 0;
  msFinPrecedente = String();
  for (size_t k = debut; k < buf.size(); k++) msFinPrecedente += (char)(buf[k] ? buf[k] : 1);  // String sans \0
}

void COta::ajoutePagesWeb(WebServer& server) {
  server.on("/update", HTTP_GET, [this, &server]() {
    if (!server.authenticate("ota", msMotDePasse.c_str())) return server.requestAuthentication();
    String html = F("<!DOCTYPE html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width'>"
      "<title>Mise à jour</title><style>body{font-family:sans-serif;margin:16px;max-width:520px}"
      "progress{width:100%;height:20px}#msg{margin-top:12px;font-weight:bold}</style></head><body>"
      "<h2>Mise à jour du firmware</h2><p>Appareil : <b>%ETIQ%</b> — version actuelle %VERSION%</p>"
      "<p>Fichier attendu : <code>firmware.bin</code> du profil de cet appareil "
      "(.pio/build/&lt;profil&gt;/firmware.bin). Un firmware d'un autre appareil est refusé.</p>"
      "<input type='file' id='f' accept='.bin'> <button onclick='envoie()'>Envoyer</button>"
      "<p><progress id='p' value='0' max='100'></progress></p><div id='msg'></div>"
      "<script>function envoie(){var f=document.getElementById('f').files[0];if(!f){return;}"
      "var fd=new FormData();fd.append('firmware',f,f.name);var x=new XMLHttpRequest();"
      "x.upload.onprogress=function(e){if(e.lengthComputable)document.getElementById('p').value=e.loaded*100/e.total;};"
      "x.onload=function(){document.getElementById('msg').textContent=x.responseText;};"
      "x.onerror=function(){document.getElementById('msg').textContent='Connexion perdue';};"
      "x.open('POST','/update');x.send(fd);document.getElementById('msg').textContent='Envoi en cours...';}"
      "</script></body></html>");
    html.replace("%ETIQ%", ETIQUETTE_FIRMWARE);
    html.replace("%VERSION%", msVersion);
    server.send(200, "text/html", html);
  });

  server.on("/update", HTTP_POST,
    // Fin de la requête : résultat, puis redémarrage si le firmware est accepté
    [this, &server]() {
      if (mbEnvoiRefuse) { server.send(400, "text/plain; charset=utf-8", "Refusé : " + msErreurEnvoi); return; }
      server.send(200, "text/plain; charset=utf-8",
                  "Firmware accepté : redémarrage. Résultat dans Home Assistant (OK ou RETOUR) d'ici 2 min.");
      delay(500);
      ESP.restart();
    },
    // Réception du fichier, par paquets
    [this, &server]() {
      HTTPUpload& up = server.upload();
      if (up.status == UPLOAD_FILE_START) {
        mbEnvoiRefuse = false; msErreurEnvoi = ""; mbEtiquetteOk = false; msEtiquetteTrouvee = ""; msFinPrecedente = "";
        if (!server.authenticate("ota", msMotDePasse.c_str())) {
          mbEnvoiRefuse = true; msErreurEnvoi = "mot de passe refusé";
          publie("ECHEC", ": mot de passe refusé (page Web)");
          return;
        }
        publie("DEBUT", "(page Web, " + up.filename + ")");
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
          mbEnvoiRefuse = true; msErreurEnvoi = String("début refusé : ") + Update.errorString();
        }
      }
      else if (up.status == UPLOAD_FILE_WRITE) {
        if (mbEnvoiRefuse) return;
        esp_task_wdt_reset();   // Réception d'un seul tenant dans handleClient() (chien de garde de loop())
        chercheEtiquette(up.buf, up.currentSize);
        if (Update.write(up.buf, up.currentSize) != up.currentSize) {
          mbEnvoiRefuse = true; msErreurEnvoi = String("écriture : ") + Update.errorString();
          Update.abort();
        }
      }
      else if (up.status == UPLOAD_FILE_END) {
        if (!mbEnvoiRefuse && !mbEtiquetteOk) {
          mbEnvoiRefuse = true;
          msErreurEnvoi = msEtiquetteTrouvee.length()
                          ? "firmware d'un autre appareil (" + msEtiquetteTrouvee + ")"
                          : String("pas d'étiquette d'appareil dans ce fichier");
          Update.abort();
        }
        if (!mbEnvoiRefuse && !Update.end(true)) {
          mbEnvoiRefuse = true; msErreurEnvoi = String("fin : ") + Update.errorString();
        }
        if (mbEnvoiRefuse) { if (msErreurEnvoi != "mot de passe refusé") publie("ECHEC", ": " + msErreurEnvoi + " (page Web)"); }
        else {
          noteMiseAJour("page Web");
          publie("TRANSFERT", ": terminé (" + String(up.totalSize) + " octets), redémarrage");
        }
      }
      else if (up.status == UPLOAD_FILE_ABORTED) {
        Update.abort();
        mbEnvoiRefuse = true; msErreurEnvoi = "transfert interrompu";
        publie("ECHEC", ": transfert interrompu (page Web)");
      }
    });
}
