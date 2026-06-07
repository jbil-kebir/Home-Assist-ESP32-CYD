#include <Arduino.h>
#include <Preferences.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <WebServer.h>
#include <PubSubClient.h>
#include <time.h>

// ─── Valeurs par défaut (utilisées si NVS vide) ───────────────────────────────
#define DEFAULT_WIFI_SSID    "Simpsonn"
#define DEFAULT_WIFI_PASSWD  "al177SOLO$*"
#define DEFAULT_MQTT_SERVER  "51.77.244.19"
#define DEFAULT_MQTT_PORT    1883
#define DEFAULT_MQTT_USER    "ubuntu"
#define DEFAULT_MQTT_PASSWD  "al177SOLO$*"

#define MQTT_TOPIC  "home/thermometre/state"
#define NTP_SERVER  "pool.ntp.org"
#define TZ_STRING   "CET-1CEST,M3.5.0,M10.5.0/3"
#define NVS_NS      "cydmon"
#define AP_SSID     "CydMonitor"

#define TFT_BL  21

// ─── Layout (paysage 320×240) ─────────────────────────────────────────────────
static constexpr int TITLE_H  = 30;
static constexpr int MAIN_Y   = TITLE_H;
static constexpr int MAIN_H   = 175;
static constexpr int STATUS_Y = MAIN_Y + MAIN_H;   // 205
static constexpr int STATUS_H = 240 - STATUS_Y;    // 35

static constexpr int DIV_X    = 163;
static constexpr int DISK_CX  = 81;
static constexpr int DISK_CY  = MAIN_Y + MAIN_H / 2;
static constexpr int DISK_R   = 60;

static constexpr int VAL_LX   = 170;
static constexpr int VAL_RX   = 314;
static constexpr int VAL_Y0   = MAIN_Y + 10;  // 40
static constexpr int VAL_DY   = 28;
static constexpr int VAL_IP_Y = VAL_Y0 + 5 * VAL_DY + 6;  // 186

// ─── Configuration (NVS) ─────────────────────────────────────────────────────
struct Config {
  String   wifiSsid   = DEFAULT_WIFI_SSID;
  String   wifiPasswd = DEFAULT_WIFI_PASSWD;
  String   mqttServer = DEFAULT_MQTT_SERVER;
  uint16_t mqttPort   = DEFAULT_MQTT_PORT;
  String   mqttUser   = DEFAULT_MQTT_USER;
  String   mqttPasswd = DEFAULT_MQTT_PASSWD;
} gConfig;

void loadConfig() {
  Preferences p;
  p.begin(NVS_NS, true);
  gConfig.wifiSsid   = p.getString("wifi_ssid",  DEFAULT_WIFI_SSID);
  gConfig.wifiPasswd = p.getString("wifi_pass",  DEFAULT_WIFI_PASSWD);
  gConfig.mqttServer = p.getString("mqtt_srv",   DEFAULT_MQTT_SERVER);
  gConfig.mqttPort   = p.getUShort("mqtt_port",  DEFAULT_MQTT_PORT);
  gConfig.mqttUser   = p.getString("mqtt_user",  DEFAULT_MQTT_USER);
  gConfig.mqttPasswd = p.getString("mqtt_pass",  DEFAULT_MQTT_PASSWD);
  p.end();
}

void saveConfig() {
  Preferences p;
  p.begin(NVS_NS, false);
  p.putString("wifi_ssid", gConfig.wifiSsid);
  p.putString("wifi_pass", gConfig.wifiPasswd);
  p.putString("mqtt_srv",  gConfig.mqttServer);
  p.putUShort("mqtt_port", gConfig.mqttPort);
  p.putString("mqtt_user", gConfig.mqttUser);
  p.putString("mqtt_pass", gConfig.mqttPasswd);
  p.end();
}

// ─── État ─────────────────────────────────────────────────────────────────────
struct {
  bool on        = false;
  bool fresh     = false;
  char lastChange[20] = "";
} gDel;

struct {
  int r=0, g=0, b=0, lux=0, brut=0;
  bool fresh = false;
} gCoul;

bool gRedrawMain = false;
bool gMqttOk     = false;
bool gApMode     = false;

// ─── Globaux ──────────────────────────────────────────────────────────────────
TFT_eSPI     tft;
WiFiClient   wifiClient;
PubSubClient mqtt(wifiClient);
WebServer    webServer(80);

// ─── Utilitaires ──────────────────────────────────────────────────────────────
static String tok(const String& s, int n) {
  int pos = 0, i = 0;
  while (true) {
    int sp = s.indexOf(' ', pos);
    if (i == n) return sp < 0 ? s.substring(pos) : s.substring(pos, sp);
    if (sp < 0) return "";
    pos = sp + 1; i++;
  }
}

static void nowStr(char* buf, size_t len) {
  struct tm t;
  if (getLocalTime(&t)) strftime(buf, len, "%d/%m/%Y %H:%M:%S", &t);
  else buf[0] = '\0';
}

static void nowTimeStr(char* buf, size_t len) {
  struct tm t;
  if (getLocalTime(&t)) strftime(buf, len, "%H:%M:%S", &t);
  else snprintf(buf, len, "--:--:--");
}


// ─── Callback MQTT ────────────────────────────────────────────────────────────
void onMqtt(char* topic, byte* payload, unsigned int len) {
  String msg;
  msg.reserve(len);
  for (unsigned int i = 0; i < len; i++) msg += (char)payload[i];
  msg.trim();

  if (tok(msg, 0) != "ThNomade") return;

  String type = tok(msg, 1);

  if (type == "Del") {
    bool newState = (tok(msg, 4) == "1");
    if (!gDel.fresh || newState != gDel.on)
      nowStr(gDel.lastChange, sizeof(gDel.lastChange));
    gDel.on     = newState;
    gDel.fresh  = true;
    gRedrawMain = true;
  } else if (type == "Coul") {
    gCoul.r     = tok(msg, 4).toInt();
    gCoul.g     = tok(msg, 5).toInt();
    gCoul.b     = tok(msg, 6).toInt();
    gCoul.lux   = tok(msg, 7).toInt();
    gCoul.brut  = tok(msg, 8).toInt();
    gCoul.fresh = true;
    gRedrawMain = true;
  }
}

// ─── Affichage ────────────────────────────────────────────────────────────────
void drawTitle() {
  tft.fillRect(0, 0, 320, TITLE_H, TFT_NAVY);
  tft.setTextFont(2);
  tft.setTextDatum(ML_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_NAVY);
  tft.drawString("ThNomade - DEL Monitor", 6, TITLE_H / 2);
}

void updateTitleClock() {
  char timeBuf[10];
  nowTimeStr(timeBuf, sizeof(timeBuf));
  tft.setTextFont(2);
  tft.setTextDatum(MR_DATUM);
  tft.setTextColor(TFT_YELLOW, TFT_NAVY);
  tft.drawString(String(timeBuf) + "  ", 314, TITLE_H / 2);
}

void drawDisk() {
  tft.fillRect(0, MAIN_Y, DIV_X, MAIN_H, TFT_BLACK);
  uint16_t dc = gDel.on ? TFT_GREEN : 0x4208;
  tft.fillCircle(DISK_CX, DISK_CY, DISK_R, dc);
  tft.drawCircle(DISK_CX, DISK_CY, DISK_R, TFT_WHITE);
}

void drawValues() {
  tft.fillRect(DIV_X + 1, MAIN_Y, 319 - DIV_X, MAIN_H, TFT_BLACK);
  tft.drawFastVLine(DIV_X, MAIN_Y, MAIN_H, TFT_DARKGREY);

  const struct { const char* lbl; uint16_t color; int val; } rows[4] = {
    { "R :",    TFT_RED,   gCoul.r    },
    { "G :",    TFT_GREEN, gCoul.g    },
    { "B :",    TFT_CYAN,  gCoul.b    },
    { "Brut :", TFT_WHITE, gCoul.brut },
  };

  tft.setTextFont(2);
  int lblColW = tft.textWidth("Brut :", 2);
  int valColW = tft.textWidth("9999", 2);
  int valRX   = VAL_LX + lblColW + 10 + valColW;  // 10px de gap explicite

  for (int i = 0; i < 4; i++) {
    int y = VAL_Y0 + i * VAL_DY;
    String valStr = gCoul.fresh ? String(rows[i].val) : "--";

    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(rows[i].color, TFT_BLACK);
    tft.drawString(rows[i].lbl, VAL_LX, y);

    tft.setTextDatum(TR_DATUM);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(valStr, valRX, y);
  }

  // Séparateur + adresse IP
  tft.drawFastHLine(DIV_X + 4, VAL_IP_Y - 5, 152, TFT_DARKGREY);
  tft.setTextFont(2);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.drawString("IP:", VAL_LX, VAL_IP_Y);
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  String ip = gApMode ? "192.168.4.1" : WiFi.localIP().toString();
  tft.drawString(ip, VAL_RX, VAL_IP_Y);
}

void drawStatusBar() {
  tft.fillRect(0, STATUS_Y, 320, STATUS_H, 0x1082);
  int cy = STATUS_Y + STATUS_H / 2;
  tft.setTextFont(2);
  tft.setTextDatum(ML_DATUM);
  tft.setTextColor(TFT_LIGHTGREY, 0x1082);
  if (gDel.fresh && gDel.lastChange[0] != '\0')
    tft.drawString("Dernier chgt: " + String(gDel.lastChange), 6, cy);
  else
    tft.drawString("En attente...", 6, cy);
  uint16_t dotColor = gMqttOk ? TFT_GREEN : TFT_RED;
  tft.fillCircle(308, cy, 6, dotColor);
  tft.setTextDatum(MR_DATUM);
  tft.setTextColor(TFT_DARKGREY, 0x1082);
  tft.drawString(gMqttOk ? "MQTT" : "---", 299, cy);
}

void drawAll() {
  tft.fillScreen(TFT_BLACK);
  drawTitle();
  updateTitleClock();
  drawDisk();
  drawValues();
  drawStatusBar();
}

void drawApScreen() {
  tft.fillScreen(TFT_BLACK);
  tft.fillRect(0, 0, 320, TITLE_H, TFT_NAVY);
  tft.setTextFont(2);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_NAVY);
  tft.drawString("CydMonitor - Configuration", 160, TITLE_H / 2);

  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString("Mode point d'acces WiFi", 160, 80);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("1. Connectez-vous au reseau :", 160, 115);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString(AP_SSID, 160, 140);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("2. Ouvrez dans un navigateur :", 160, 170);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("http://192.168.4.1", 160, 195);
}

// ─── Interface Web ────────────────────────────────────────────────────────────
static const char HTML_PAGE[] = R"html(
<!DOCTYPE html><html><head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>CydMonitor</title>
<style>
  body{font-family:sans-serif;max-width:420px;margin:30px auto;padding:0 16px;background:#f4f4f4}
  h1{color:#003366;margin-bottom:2px}
  h2{color:#555;font-size:.9em;border-bottom:1px solid #ccc;padding-bottom:3px;margin-top:18px}
  label{display:block;margin-top:8px;font-size:.85em;font-weight:bold;color:#444}
  input{width:100%;padding:7px;box-sizing:border-box;border:1px solid #ccc;border-radius:4px}
  button{margin-top:22px;width:100%;padding:11px;background:#003366;color:#fff;
         border:none;border-radius:4px;font-size:1em;cursor:pointer}
  button:hover{background:#0055aa}
  .ip{color:#999;font-size:.8em;margin:2px 0 10px}
</style></head><body>
<h1>CydMonitor</h1>
<p class="ip">IP : %IP%</p>
<form method="POST" action="/save">
  <h2>WiFi</h2>
  <label>SSID</label>
  <input name="wifi_ssid" value="%WIFI_SSID%">
  <label>Mot de passe</label>
  <input name="wifi_pass" type="password" value="%WIFI_PASS%">
  <h2>MQTT</h2>
  <label>Serveur</label>
  <input name="mqtt_srv" value="%MQTT_SRV%">
  <label>Port</label>
  <input name="mqtt_port" type="number" value="%MQTT_PORT%">
  <label>Utilisateur</label>
  <input name="mqtt_user" value="%MQTT_USER%">
  <label>Mot de passe</label>
  <input name="mqtt_pass" type="password" value="%MQTT_PASS%">
  <button type="submit">Enregistrer et redemarrer</button>
</form>
</body></html>
)html";

void handleRoot() {
  String page = HTML_PAGE;
  page.replace("%IP%",        gApMode ? "192.168.4.1" : WiFi.localIP().toString());
  page.replace("%WIFI_SSID%", gConfig.wifiSsid);
  page.replace("%WIFI_PASS%", gConfig.wifiPasswd);
  page.replace("%MQTT_SRV%",  gConfig.mqttServer);
  page.replace("%MQTT_PORT%", String(gConfig.mqttPort));
  page.replace("%MQTT_USER%", gConfig.mqttUser);
  page.replace("%MQTT_PASS%", gConfig.mqttPasswd);
  webServer.send(200, "text/html", page);
}

void handleSave() {
  if (webServer.hasArg("wifi_ssid")) gConfig.wifiSsid   = webServer.arg("wifi_ssid");
  if (webServer.hasArg("wifi_pass")) gConfig.wifiPasswd = webServer.arg("wifi_pass");
  if (webServer.hasArg("mqtt_srv"))  gConfig.mqttServer = webServer.arg("mqtt_srv");
  if (webServer.hasArg("mqtt_port")) gConfig.mqttPort   = (uint16_t)webServer.arg("mqtt_port").toInt();
  if (webServer.hasArg("mqtt_user")) gConfig.mqttUser   = webServer.arg("mqtt_user");
  if (webServer.hasArg("mqtt_pass")) gConfig.mqttPasswd = webServer.arg("mqtt_pass");
  saveConfig();
  webServer.send(200, "text/html",
    "<html><head><meta charset='UTF-8'></head><body>"
    "<h2 style='font-family:sans-serif;color:#003366'>Configuration sauvegardee.</h2>"
    "<p style='font-family:sans-serif'>Redemarrage en cours...</p>"
    "</body></html>");
  delay(1500);
  ESP.restart();
}

// ─── Connexion WiFi ───────────────────────────────────────────────────────────
bool connectWifi() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextFont(2);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Connexion WiFi : " + gConfig.wifiSsid, 160, 100);

  WiFi.mode(WIFI_STA);
  WiFi.begin(gConfig.wifiSsid.c_str(), gConfig.wifiPasswd.c_str());

  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 15000) delay(300);

  if (WiFi.status() != WL_CONNECTED) {
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.drawString("WiFi ECHEC", 160, 125);
    delay(1200);
    return false;
  }

  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawString("WiFi OK  " + WiFi.localIP().toString(), 160, 125);

  configTime(0, 0, NTP_SERVER);
  setenv("TZ", TZ_STRING, 1);
  tzset();
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString("Synchronisation NTP...", 160, 150);
  struct tm t;
  t0 = millis();
  while (!getLocalTime(&t) && millis() - t0 < 5000) delay(200);

  delay(800);
  return true;
}

void startApMode() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID);
  gApMode = true;
  drawApScreen();
}

// ─── Connexion MQTT (non-bloquant) ────────────────────────────────────────────
void reconnectMqtt() {
  if (mqtt.connected()) return;
  static unsigned long last = 0;
  if (millis() - last < 5000) return;
  last = millis();

  gMqttOk = false;
  String id = "CydDel_" + String(random(0xffff), HEX);
  if (mqtt.connect(id.c_str(), gConfig.mqttUser.c_str(), gConfig.mqttPasswd.c_str())) {
    mqtt.subscribe(MQTT_TOPIC);
    gMqttOk = true;
    Serial.println("MQTT OK, abonne a " MQTT_TOPIC);
  } else {
    Serial.printf("MQTT echec rc=%d\n", mqtt.state());
  }
  drawStatusBar();
}

// ─── Arduino ──────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);

  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  tft.init();
  tft.setRotation(1);

  loadConfig();

  if (connectWifi()) {
    mqtt.setServer(gConfig.mqttServer.c_str(), gConfig.mqttPort);
    mqtt.setCallback(onMqtt);
    webServer.on("/",     HTTP_GET,  handleRoot);
    webServer.on("/save", HTTP_POST, handleSave);
    webServer.begin();
    drawAll();
    reconnectMqtt();
  } else {
    startApMode();
    webServer.on("/",     HTTP_GET,  handleRoot);
    webServer.on("/save", HTTP_POST, handleSave);
    webServer.begin();
  }
}

void loop() {
  webServer.handleClient();

  if (gApMode) return;

  if (WiFi.status() != WL_CONNECTED) {
    if (!connectWifi()) { startApMode(); return; }
    drawAll();
  }

  reconnectMqtt();
  mqtt.loop();

  static unsigned long lastClock = 0;
  if (millis() - lastClock >= 1000) {
    lastClock = millis();
    updateTitleClock();
  }

  if (gRedrawMain) {
    gRedrawMain = false;
    drawDisk();
    drawValues();
    drawStatusBar();
  }
}
