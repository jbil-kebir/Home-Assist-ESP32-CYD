// CDetecteurRGB_TCS34725.h
#ifndef __DETECTEUR_RGB_TCS34725__
#define __DETECTEUR_RGB_TCS34725__

#include "global.h"
#include <functional>
#include <Preferences.h>
#include <Arduino.h>
#include <Wire.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_TCS34725.h>
#include "MyDateTime.h"
#include "MyRCSwitch.h"

#define DEFAULT_CAPTEUR_RGB_TCS34725_ID (random(0, 256))  // Aléatoire

#define TIMEOUT_WIRE    30000UL // Timeout de connexion Wire
#define TIMEOUT_DHT    30000UL // Timeout de connexion Wire

// Structure utilisée par handleMqttCommand()
struct MQTT_COMMAND_33 {
  String sExpediteur="";
  String sCommand="";
  String sArg="";
};


class CDetecteurRGB_TCS34725 {
private:
  Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

  bool _initialized = false;
  unsigned char mucSdaPin =  DEFAULT_SDA_PIN;
  unsigned char mucSclPin =  DEFAULT_SCL_PIN;
  #ifdef LED_CAPTEUR_RGB
  unsigned char mucLedCapteurRGBPin =  LED_CAPTEUR_RGB_PIN;
  unsigned char mucLedRGBIsOFF = LOW;
  #endif
  unsigned char mucIntegrationTime = TCS34725_INTEGRATIONTIME_614MS; // 614 ms = max sensibilité
  unsigned char mucGain = TCS34725_GAIN_60X; // Gain X60 = max
  unsigned long mulIntervalleMesure = 10L, mulDefaultIntervalleMesure = 10L; // en minutes : Intervalle entre deux mesures. En secondes pour les tests
  // en minutes : Intervalle de forcage de la rmontée de mesure, même si la valeur n'a pas changé. En secondes pour les tests
  unsigned long mulIntervalleForcageRemonteeMesure = 20L, mulDefaultIntervalleForcageRemonteeMesure = 20L; 
  unsigned char mucNbEnvois = 1; // Nombre de fois qu'il faut remonter les mesures à chaque réveil
  unsigned long mulIntervalleEnvoi = 10L, mulDefaultIntervalleEnvoi = 10L; // en secondes : Intervalle entre deux envois. 
  bool mbForce = true; // On force une remontée à l'initialisation
  bool remonterMesuresBrutes = false; // Si actif, on remonte la couleur et les lux de la del. Sinon on remonte juste l'état ON/OFF
  Preferences prefs;
  // On mémorise la température pour n'afficher que les changements
  //unsigned long lastRgb = 0x0UL;  // Valeur invalide par défaut. Une couleur valide sera de la forme 0xFFRRGGBB
  //unsigned long newRgb = 0x0UL;  // Valeur invalide par défaut
  unsigned int muiLastR = 0;
  unsigned int muiLastG = 0;
  unsigned int muiLastB = 0;
  unsigned int muiNewR = 0;
  unsigned int muiNewG = 0;
  unsigned int muiNewB = 0;
  
  // On mémorise la température pour n'afficher que les changements
  unsigned int muiLastLux = 0x0;  // Valeur invalide par défaut. Une luminosité sera envoyée sous la forme 0xFFFFFFLM (LM = Luminosité)
  unsigned int muiNewLux = 0x0;  // Valeur invalide par défaut
  // On mémorise la température pour n'afficher que les changements
  unsigned int muiLastBrutLux = 0x0;  // Valeur invalide par défaut. Une luminosité sera envoyée sous la forme 0xFFFFFFLM (LM = Luminosité)
  unsigned int muiNewBrutLux = 0x0;  // Valeur invalide par défaut
  // Constantes et variables permettant de prendre la décision : ON ou OFF en fonction de la couleur de la del rouge
  // Les valeurs mulRougeOn et mulRougeOff sont expérimentales.
  #define VAL_LED_ROUGE_ON  210 // 
  #define VAL_LED_ROUGE_OFF  100 // 
  #define VAL_LUX_ON  1210 // 
  #define VAL_LUX_OFF  1500 // 
  unsigned int muiRougeOn = VAL_LED_ROUGE_ON;
  unsigned int muiRougeOff = VAL_LED_ROUGE_OFF;
  unsigned int muiLuxOn = VAL_LUX_ON;
  unsigned int muiLuxOff = VAL_LUX_OFF;
  bool mbNewEtatOnOff = false; // Mis à jour par calcul. N'a pas besoin d'être dans le NVS. true = ON
  bool mbLastEtatOnOff = false; // Mis à jour par calcul. N'a pas besoin d'être dans le NVS. true = ON

  const char* default_domotique_topic_prefix = "home/";
  const char* nvs_namespace = NVS_NAME_SPACE;  // 
  String mPrefixNVS = "thRgb_";  // Préfixe par défaut
  CMyDateTime *mDateTime=nullptr;
  unsigned char mucCapteurID=0;
  unsigned int mDelai_Inter_Envoi = 350; // A mettre dans NVS et page Web
  

public:
    CDetecteurRGB_TCS34725(CMyDateTime& dateTime,
        std::function<int(const char*, const char*)> cbonMqttPublish = nullptr,
        std::function<int(const char*)> cbonLoraP2PPublish = nullptr) : 
        mDateTime(&dateTime), 
        onMqttPublish(cbonMqttPublish), onLoraP2PPublish(cbonLoraP2PPublish) 
        {}

    String domotique_prefix;
    String nomEquipement = "thEquipement";
    String mqttSubTopic = "thermometre";
    bool active = true;
    bool mbMesureRemontee = false; // Permet d'empêcher le deep sleep tant qu'une mesure n'a pas été remontée
    String mqttSubTopicCommand;
    String mqttSubTopicState;
    #ifdef _RCSWITCH_MODE_
    CMyRCSwitch *mRCSwitch = nullptr;
    #endif
    // MQTT callback
    std::function<int(const char*, const char*)> onMqttPublish;    
    void setMqttPublishCallback(std::function<int(const char* topic, const char* payload)> cbMqttPublish); // Pour publication MQTT
    // Lora P2P callback
    std::function<int(const char*)> onLoraP2PPublish;    
    void setLoraP2PPublishCallback(std::function<int(const char* payload)> cbLoraP2PPublish); // Pour publication Lora P2O

    void loadFromNVS();
    void loadFromWebServer (WebServer& server);
    void saveToNVS();
    void setActive(bool state);
    void setPrefixNVS(const char* pr) {mPrefixNVS = pr;}
    String getHTML();


    bool begin(const String pref);
    int loop();
    bool read();
    // Variante avec retry (utile en low-power, car parfois le premier read échoue)
    bool readWithRetry(uint8_t retries = 3);

    void print() const;
    unsigned int getLastR() const {return muiLastR;}
    unsigned int getLastG() const {return muiLastG;}
    unsigned int getLastB() const {return muiLastB;}
    unsigned int getLastLuminosite() const {return muiLastLux;}
    unsigned int getLastLuminositeBrute() const {return muiLastBrutLux;}    

    int handleMqttCommand(const String& payload);
    bool publieSurMqttBrut(bool force=false);
    bool publieParLoraP2PBrut(bool force=false);
    bool publieSurMqttOnOff(bool force=false);
    bool publieParLoraP2POnOff(bool force=false);
    bool publieParCC1101(bool force=false);
    uint8_t computeCRC(const unsigned long* codes, int count);
    int readAndPublish(bool force=false, bool mesurer=false);
    int readAndPublishTEST(float t, float h); // Pour les tests

};
#endif // __DETECTEUR_RGB_TCS34725__
