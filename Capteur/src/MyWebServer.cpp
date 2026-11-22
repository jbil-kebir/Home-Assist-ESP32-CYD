#include <WiFi.h>
#include <PubSubClient.h>
#include "global.h"
#include "MyConfig.h"
#include "MyMqtt.h"
#include "MyWebServer.h"
#include "MyVersion.h"


void MyWebServer::handleRoot() {
  String html =
    "<!DOCTYPE html>"
    "<html lang=\"fr\">"
    "<head>"
      "<meta charset=\"UTF-8\">"
      "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
      "<title>Config " + config.nomEquipement + "</title>"
      "<style>"
        "body { font-family: Arial, sans-serif; max-width: 800px; margin: 20px auto; padding: 20px; background: #f0f0f0; }"
        "h1 { text-align: center; color: #333; }"
        "h2, h3 { margin-top: 30px; color: #444; }"
        "form { background: white; padding: 20px; border-radius: 10px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }"
        "label { font-weight: bold; margin: 8px 0 4px; display: block; }"
        "input[type=text], input[type=number], input[type=password] { "
          "padding: 8px; box-sizing: border-box; font-size: 16px; border: 1px solid #ccc; border-radius: 4px; }"
        "input[type=checkbox] { margin-right: 8px; transform: scale(1.2); }"
        "input[type=submit] { width: 100%; padding: 15px; background: #007BFF; color: white; font-size: 18px; border: none; border-radius: 5px; cursor: pointer; margin-top: 30px; }"
        "input[type=submit]:hover { background: #0056b3; }"
        ".control { text-align: center; margin: 30px 0; }"
        ".btn { padding: 18px 35px; font-size: 18px; color: white; border: none; border-radius: 8px; cursor: pointer; margin: 8px; }"
        ".btn-on { background: #28a745; }"
        ".btn-off { background: #dc3545; }"
        ".btn-orange { background: #fd7e14; }"
        ".btn-yellow { background: #ffc107; color: #212529; }"
        ".btn-cyan { background: #17a2b8; }"
        ".status { font-size: 18px; text-align: center; margin: 15px 0; font-weight: bold; }"
        ".row { display: flex; flex-wrap: wrap; gap: 10px; margin-bottom: 10px; align-items: center; }"
        ".row > div { flex: 1; min-width: 200px; }"
        ".row input[type=text], .row input[type=number] { width: 100%; }"
        ".checkbox-row { display: flex; align-items: center; gap: 10px; }"
        ".device { border: 1px solid #ddd; padding: 15px; border-radius: 8px; background: #f9f9f9; margin-bottom: 20px; }"
        ".timing-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(100px, 1fr)); gap: 8px; margin: 15px 0; }"
        ".mq-r { color: #fd7e14; cursor: help; } .mq-m { color: #007BFF; cursor: help; }"
        ".legende { text-align: center; color: #555; margin: 10px 0; }"
      "</style>"
    "</head>"
    "<body>"
      "<h1>Configuration " + config.nomEquipement + "</h1>"
      "<p style=\"text-align:center; color:#666; margin-top:-10px;\">Version " + String(VERSION) +
        " — compilée le " + versionDateCompilation().substring(0, 10) + " à " + versionDateCompilation().substring(11) + "</p>"
      "<div class=\"control\"><a href=\"/logs\" class=\"btn btn-cyan\" style=\"text-decoration:none;\">Journal</a></div>"

      "<hr style=\"margin: 40px 0;\">"

      "<form action=\"/save\" method=\"POST\">"
      "<p class=\"legende\">" LEGENDE_MARQUES "</p>";

  // === WiFi (2 colonnes) ===
  html +=  "<h2>WiFi</h2>"
        "<div class=\"device-grid\">";
      for(int i=0; i<MAX_WIFI_NETS; i++) {
        html +=  mWifi[i].getHTML(i);
      }


  html +=  "</div>";

  // === MQTT (2 colonnes) ===
  #ifndef __DESACTIVE_ENVOI_MQTT__
  html += mqtt.getHTML();
  #endif

  // === PRÉFIXE DOMOTIQUE ===
  /*html += "<h2>Préfixe domotique (projecteur, guirlande, sdb)</h2>"
        "<label>Préfixe (ex: home/)</label>"
        "<input type=\"text\" name=\"domo_prefix\" value=\"" + config.domotique_prefix + "\">";*/
  html += config.getHTML();

  // === APPAREILS 433 MHz ===
  html +=  "<h2>Appareils</h2>"
        "<div class=\"device-grid\">";

  // === Thermomètre local ===
  #ifdef CAPTEUR_DS18B20
  if (config.ds18b20 != nullptr) html += config.ds18b20->getHTML();
  #endif
  #ifdef CAPTEUR_DHT20
  if (config.dht20 != nullptr) html += config.dht20->getHTML();
  #endif
  #ifdef FLOTTEUR_VERTICAL
  if (config.mFlotteurVertical != nullptr) html += config.mFlotteurVertical->getHTML();
  #endif
  #ifdef CAPTEUR_RGB_TCS34725
  if (config.mCapteurRGB != nullptr) html += config.mCapteurRGB->getHTML();
  #endif
  #ifdef CAPTEUR_MICRO_MAX4466
  if (config.mMicro != nullptr) html += config.mMicro->getHTML();
  #endif
  #ifdef ACTIONNEUR_IR
  if (config.mActionneurIR != nullptr) html += config.mActionneurIR->getHTML();
  #endif
  // === Batterie ===
  #ifdef CAPTEUR_BATTERIE
  if (config.mBatterieAA != nullptr) html += config.mBatterieAA->getHTML();
  #endif
  #ifdef CAPTEUR_JOUR_NUIT
  if (config.mJourNuit != nullptr) html += config.mJourNuit->getHTML();
  #endif
  #ifdef CAPTEUR_PRESENCE_RESEAU
  if (config.mPresence != nullptr) html += config.mPresence->getHTML();
  #endif

  html +=  "</div>";

  // 5.3 : enregistrement à chaud (redémarrage seulement si le Wi-Fi, un topic, une broche ou l'activation d'un capteur
  // change) ; redémarrage à la demande par le second bouton
  html +=  "<p class=\"legende\">" LEGENDE_MARQUES "</p>"
    "<input type=\"submit\" value=\"Sauvegarder\">"
    "</form>"
    "<form method=\"post\" action=\"/reboot\" onsubmit=\"return confirm('Redémarrer le nœud ?');\">"
    "<input type=\"submit\" value=\"Redémarrer\" style=\"background:#c0392b\">"
    "</form>"
  "</body>"
  "</html>";

  server.send(200, "text/html", html);
}


//----------------------------------------------------------------------------------
// Réglages à chaud (Capteur 5.3) : empreintes des réglages qui demandent un redémarrage (Wi-Fi, topics, broches,
// activation des capteurs, puce configurée) ou une reconnexion MQTT (broker, identifiants, nom), comparées avant /
// après la lecture de la page. Les autres valeurs (sommeil, forçages, délais d'alerte, seuils, mute…) sont appliquées
// dès leur lecture. Dans le doute, un réglage va dans la catégorie redémarrage.
//----------------------------------------------------------------------------------
struct EmpreintesReglages { String sWifi, sNoeud, sMqtt; };

static EmpreintesReglages empreintesReglages(CConfig& config, CWifi* mWifi) {
  EmpreintesReglages e;
  for (int i = 0; i < MAX_WIFI_NETS; i++) e.sWifi += mWifi[i].reglagesRedemarrage() + "\x1e";
  e.sNoeud = config.reglagesRedemarrage() + "\x1e";
  #ifdef CAPTEUR_DS18B20
  if (config.ds18b20 != nullptr) e.sNoeud += config.ds18b20->reglagesRedemarrage() + "\x1e";
  #endif
  #ifdef CAPTEUR_DHT20
  if (config.dht20 != nullptr) e.sNoeud += config.dht20->reglagesRedemarrage() + "\x1e";
  #endif
  #ifdef FLOTTEUR_VERTICAL
  if (config.mFlotteurVertical != nullptr) e.sNoeud += config.mFlotteurVertical->reglagesRedemarrage() + "\x1e";
  #endif
  #ifdef CAPTEUR_RGB_TCS34725
  if (config.mCapteurRGB != nullptr) e.sNoeud += config.mCapteurRGB->reglagesRedemarrage() + "\x1e";
  #endif
  #ifdef CAPTEUR_MICRO_MAX4466
  if (config.mMicro != nullptr) e.sNoeud += config.mMicro->reglagesRedemarrage() + "\x1e";
  #endif
  #ifdef ACTIONNEUR_IR
  if (config.mActionneurIR != nullptr) e.sNoeud += config.mActionneurIR->reglagesRedemarrage() + "\x1e";
  #endif
  #ifdef CAPTEUR_BATTERIE
  if (config.mBatterieAA != nullptr) e.sNoeud += config.mBatterieAA->reglagesRedemarrage() + "\x1e";
  #endif
  #ifdef CAPTEUR_JOUR_NUIT
  if (config.mJourNuit != nullptr) e.sNoeud += config.mJourNuit->reglagesRedemarrage() + "\x1e";
  #endif
  #ifdef CAPTEUR_PRESENCE_RESEAU
  if (config.mPresence != nullptr) e.sNoeud += config.mPresence->reglagesRedemarrage() + "\x1e";
  #endif
  e.sMqtt = config.reglagesConnexion();   // + broker, ajouté par handleSave() (membre mqtt)
  return e;
}

// Page de retour d'un enregistrement ou d'un redémarrage, retour à la page principale
static String pageRetour(const String& sTitre, const String& sMessage, int iDelaiS) {
  return "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><meta http-equiv=\"refresh\" content=\"" + String(iDelaiS) +
         ";url=/\"><title>Sauvegarde</title><style>body{font-family:Arial;text-align:center;padding:50px;background:#f0f0f0;}"
         "h1{color:#007BFF;font-size:40px;}p{font-size:22px;}</style></head><body><h1>" + sTitre + "</h1><p>" + sMessage +
         "<br>Retour à la page principale dans " + String(iDelaiS) + " secondes.</p></body></html>";
}

// Bouton « Redémarrer » (5.3) : indépendant des modifications
void MyWebServer::handleReboot() {
  server.send(200, "text/html", pageRetour("Redémarrage", "Redémarrage du nœud en cours...", 10));
  delay(1000);
  ESP.restart();
}

void MyWebServer::handleSave() {
  EmpreintesReglages avant = empreintesReglages(config, mWifi);
  #ifndef __DESACTIVE_ENVOI_MQTT__
  avant.sMqtt += "\x1e" + mqtt.reglagesConnexion();
  #endif

  // === PARAMÈTRES CLASSIQUES ===
  config.loadFromWebServer(server);

  // === PARAMÈTRES WIFI ===
  for(int i=0; i<MAX_WIFI_NETS;i++) {
    mWifi[i].loadFromWebServer(server);
    Serial.println("MyWebServer::handleSave() - mWifi[" + String(i) + "].print();");
    mWifi[i].print();
  }
  
  // === PARAMÈTRES MQTT ===
  #ifndef __DESACTIVE_ENVOI_MQTT__
  mqtt.loadFromWebServer(server);
  #endif

  #ifdef CAPTEUR_DS18B20
  if (config.ds18b20 != nullptr) config.ds18b20->loadFromWebServer(server);
  #endif
  #ifdef CAPTEUR_DHT20
  if (config.dht20 != nullptr) config.dht20->loadFromWebServer(server);
  #endif
  #ifdef FLOTTEUR_VERTICAL
  if (config.mFlotteurVertical != nullptr) config.mFlotteurVertical->loadFromWebServer(server);
  #endif
  #ifdef CAPTEUR_RGB_TCS34725
  if (config.mCapteurRGB != nullptr) config.mCapteurRGB->loadFromWebServer(server);
  #endif
  #ifdef CAPTEUR_MICRO_MAX4466
  if (config.mMicro != nullptr) config.mMicro->loadFromWebServer(server);
  #endif
  #ifdef ACTIONNEUR_IR
  if (config.mActionneurIR != nullptr) config.mActionneurIR->loadFromWebServer(server);
  #endif
  #ifdef CAPTEUR_BATTERIE
  if (config.mBatterieAA != nullptr) config.mBatterieAA->loadFromWebServer(server);
  #endif
  #ifdef CAPTEUR_JOUR_NUIT
  if (config.mJourNuit != nullptr) config.mJourNuit->loadFromWebServer(server);
  #endif
  #ifdef CAPTEUR_PRESENCE_RESEAU
  if (config.mPresence != nullptr) config.mPresence->loadFromWebServer(server);
  #endif

  // === SAUVEGARDE DES PARAMETRES GENERAUX ===
  config.saveToNVS();
  // === SAUVEGARDE DES PARAMETRES WIFI ===
  for(int i=0; i<MAX_WIFI_NETS;i++) {
    mWifi[i].saveToNVS();
  }
  // === SAUVEGARDE DES PARAMETRES MQTT ===
  #ifndef __DESACTIVE_ENVOI_MQTT__
  mqtt.saveToNVS();
  #endif
  
  // === SAUVEGARDE DU THERMOMETRE LOCAL ===
  #ifdef CAPTEUR_DS18B20
  if (config.ds18b20 != nullptr) config.ds18b20->saveToNVS();
  #endif
  #ifdef CAPTEUR_DHT20
  if (config.dht20 != nullptr) config.dht20->saveToNVS();
  #endif
  #ifdef FLOTTEUR_VERTICAL
  if (config.mFlotteurVertical != nullptr) config.mFlotteurVertical->saveToNVS();
  #endif
  #ifdef CAPTEUR_RGB_TCS34725
  if (config.mCapteurRGB != nullptr) config.mCapteurRGB->saveToNVS();
  #endif
  #ifdef CAPTEUR_MICRO_MAX4466
  if (config.mMicro != nullptr) config.mMicro->saveToNVS();
  #endif
  #ifdef ACTIONNEUR_IR
  if (config.mActionneurIR != nullptr) config.mActionneurIR->saveToNVS();
  #endif
  #ifdef CAPTEUR_BATTERIE
  if (config.mBatterieAA != nullptr) config.mBatterieAA->saveToNVS();
  #endif
  #ifdef CAPTEUR_JOUR_NUIT
  if (config.mJourNuit != nullptr) config.mJourNuit->saveToNVS();
  #endif
  #ifdef CAPTEUR_PRESENCE_RESEAU
  if (config.mPresence != nullptr) config.mPresence->saveToNVS();
  #endif

  // 5.3 : redémarrage seulement si un réglage qui l'exige a changé ; sinon reconnexion MQTT ou rien (appliqué à chaud)
  EmpreintesReglages apres = empreintesReglages(config, mWifi);
  #ifndef __DESACTIVE_ENVOI_MQTT__
  apres.sMqtt += "\x1e" + mqtt.reglagesConnexion();
  #endif
  String sRaison = (apres.sWifi != avant.sWifi) ? "Wi-Fi modifié"
                 : (apres.sNoeud != avant.sNoeud) ? "topic, broche ou capteur modifié" : "";
  if (sRaison.length() > 0) {
    Serial.println("MyWebServer::handleSave() - Enregistré, redémarrage (" + sRaison + ")");
    server.send(200, "text/html", pageRetour("✓ Configuration sauvegardée", "Redémarrage (" + sRaison + ")...", 10));
    delay(1000);
    ESP.restart();
  }
  String sMessage = "Enregistré, appliqué sans redémarrage";
  #ifndef __DESACTIVE_ENVOI_MQTT__
  if (apres.sMqtt != avant.sMqtt) {
    sMessage = "Enregistré, reconnexion MQTT";
    mqtt.reconnecteNouveauxReglages();
  }
  #endif
  Serial.println("MyWebServer::handleSave() - " + sMessage);
  // Nouveaux réglages renvoyés (sommeil, forçages…) : HA les voit sans attendre un GET
  config.envoieConfig();
  server.send(200, "text/html", pageRetour("✓ Configuration sauvegardée", sMessage, 3));
}


void MyWebServer::handleNotFound() {
  server.send(404, "text/plain", "Not found");
}

//------------------------------------------------------------------------------
// Journal : page /logs, rafraîchie toutes les 3 s depuis /logs/data (voir MyJournal.h).
// Filtre / Masquer : tri des lignes dans le navigateur (mémorisé dans le navigateur) ; le journal garde tout.
//------------------------------------------------------------------------------
void MyWebServer::handleLogs() {
  String html = F(
    "<!DOCTYPE html><html lang=\"fr\"><head>"
    "<meta charset=\"UTF-8\">"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
    "<title>Journal %NOM%</title>"
    "<style>"
    "body{font-family:Arial,sans-serif;max-width:900px;margin:20px auto;padding:20px;background:#f0f0f0;}"
    "h1{text-align:center;color:#333;}"
    ".controls{display:flex;gap:10px;margin-bottom:12px;align-items:center;flex-wrap:wrap;}"
    ".btn{padding:10px 22px;font-size:15px;color:white;border:none;border-radius:6px;cursor:pointer;text-decoration:none;display:inline-block;}"
    ".btn-gray{background:#6c757d;}.btn-red{background:#dc3545;}.btn-orange{background:#fd7e14;}"
    ".champ{flex:1;min-width:160px;padding:8px;font-size:14px;border:1px solid #ccc;border-radius:6px;}"
    "#logbox{background:#1e1e1e;color:#d4d4d4;font-family:monospace;font-size:13px;"
    "padding:15px;border-radius:8px;height:500px;overflow-y:auto;white-space:pre-wrap;word-break:break-all;}"
    "#status{font-size:12px;color:#888;margin-top:6px;display:flex;justify-content:space-between;}"
    "label{display:flex;align-items:center;gap:6px;cursor:pointer;font-size:15px;}"
    ".dbg{background:#fff;border:1px solid #ddd;border-radius:6px;padding:8px 12px;font-size:14px;}"
    ".dbg label{font-size:14px;}"
    "</style></head><body>"
    "<h1>Journal %NOM%</h1>"
    "<p style=\"text-align:center;color:#666;margin-top:-10px;\">Version %VERSION%</p>"
    "<div class=\"controls\">"
    "<a href=\"/\" class=\"btn btn-gray\">Retour</a>"
    "<button type=\"button\" id=\"pause\" class=\"btn btn-gray\">Pause</button>"
    "<form action=\"/logs/clear\" method=\"POST\" style=\"margin:0\">"
    "<button type=\"submit\" class=\"btn btn-red\">Vider</button></form>"
    "<label><input type=\"checkbox\" id=\"autoscroll\" checked> Auto-scroll</label>"
    "</div>"
    "<div class=\"controls\">"
    "<input type=\"text\" id=\"filtre\" class=\"champ\" placeholder=\"Filtre : lignes contenant… (a|b)\">"
    "<input type=\"text\" id=\"masque\" class=\"champ\" placeholder=\"Masquer : lignes contenant… (a|b)\">"
    "</div>"
  );
  html += F(
    "<div id=\"logbox\">(chargement...)</div>"
    "<div id=\"status\"><span id=\"nb\"></span><span id=\"maj\"></span></div>"
    "<script>"
    "const box=document.getElementById('logbox'),maj=document.getElementById('maj'),nb=document.getElementById('nb');"
    "const as=document.getElementById('autoscroll'),fi=document.getElementById('filtre'),ma=document.getElementById('masque'),pb=document.getElementById('pause');"
    "let brut='',pause=false;"
    "function lire(k){try{return localStorage.getItem(k)||'';}catch(e){return '';}}"
    "function ecrire(k,v){try{localStorage.setItem(k,v);}catch(e){}}"
    "fi.value=lire('jrnl_filtre');ma.value=lire('jrnl_masque');"
    "function motifs(s){return s.toLowerCase().split('|').map(x=>x.trim()).filter(x=>x);}"
    "function affiche(){"
    "const f=motifs(fi.value),m=motifs(ma.value);"
    "const lignes=brut.split('\\n').filter(l=>l);"
    "const g=lignes.filter(l=>{const b=l.toLowerCase();return (!f.length||f.some(x=>b.includes(x)))&&!m.some(x=>b.includes(x));});"
    "box.textContent=g.join('\\n');"
    "if(as.checked)box.scrollTop=box.scrollHeight;"
    "nb.textContent=g.length+' / '+lignes.length+' lignes';"
    "}"
    "fi.oninput=function(){ecrire('jrnl_filtre',fi.value);affiche();};"
    "ma.oninput=function(){ecrire('jrnl_masque',ma.value);affiche();};"
    "pb.onclick=function(){pause=!pause;pb.textContent=pause?'Reprendre':'Pause';pb.className='btn '+(pause?'btn-orange':'btn-gray');if(!pause)refresh();};"
    "async function refresh(){"
    "if(pause)return;"
    "try{"
    "const r=await fetch('/logs/data');"
    "brut=await r.text();affiche();"
    "maj.textContent='Mis à jour : '+new Date().toLocaleTimeString();"
    "}catch(e){maj.textContent='Erreur de connexion';}"
    "}"
    "refresh();setInterval(refresh,3000);"
    "</script></body></html>"
  );
  // Nom de l'équipement et version : plusieurs journaux ouverts côte à côte restent identifiables
  html.replace("%NOM%", config.nomEquipement);
  html.replace("%VERSION%", String(VERSION) + " — compilée le " + versionDateCompilation().substring(0, 10) + " à " + versionDateCompilation().substring(11));
  server.send(200, "text/html", html);
}

void MyWebServer::handleLogsData() {
  server.send(200, "text/plain; charset=utf-8", gJournal.getContenu());
}

void MyWebServer::handleLogsClear() {
  gJournal.vide();
  server.send(200, "text/html",
    F("<html><head><meta http-equiv='refresh' content='0;url=/logs'></head><body></body></html>"));
}

void MyWebServer::setup() {
  server.on("/", HTTP_GET, [this]() { handleRoot(); });
  server.on("/save", HTTP_POST, [this]() { handleSave(); });
  server.on("/reboot", HTTP_POST, [this]() { handleReboot(); });
  server.on("/logs",       HTTP_GET,  [this]() { handleLogs(); });
  server.on("/logs/data",  HTTP_GET,  [this]() { handleLogsData(); });
  server.on("/logs/clear", HTTP_POST, [this]() { handleLogsClear(); });
  server.onNotFound([this]() { handleNotFound(); });
  server.begin();
  Serial.printf("Serveur web démarré sur %s\n", WiFi.localIP().toString().c_str()); Serial.flush();
}

void MyWebServer::loop() {
  server.handleClient();
}