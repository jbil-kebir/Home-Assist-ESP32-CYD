#ifndef __GLOBAL_H__
#define __GLOBAL_H__

//================ ESP32C3 ================
//          MISO - GPIO5   ||   5V
//          MOSI - GPIO6   ||   GND
//          SS   - GPIO7   ||   3.3V
//          SDA  - GPIO8   ||   GPIO4 -  SCK
//          SCL  - GPIO9   ||   GPIO3
//                 GPIO10  ||   GPIO2
//          RX   - GPIO20  ||   GPIO1
//          TX   - GPIO21  ||   GPIO0

//================ ESP32S3 ================
//               - GPIO1   ||   5V
//               - GPIO2   ||   GND
//               - GPIO3   ||   3.3V
//               - GPIO4   ||   GPIO9  -  MOSI
//          SDA  - GPIO5   ||   GPIO8  -  MISO
//          SCL  - GPIO6   ||   GPIO7  -  SCK
//          TX   - GPIO43  ||   GPIO44 -  RX


// Envoi des valeurs de test. Voir main.cpp : 
//   static float mes[nbMes][2] = {
//                          {10.0,90.0},
//                          {20.0,80.0},
//                          {40.0,60.0},
//                          {-12.0,2.0}
//                        };
//#define TEST_EMISSION_RCS


//#define DEBUG_NO_DEEP_SLEEP
//#define _RCSWITCH_MODE_ // Envoi par RCSwitch possible si décommentée
//#define _WIFI_MODE_ // Envoi par WIFI possible si décommentée
#define __DESACTIVE_ENVOI_MQTT__ // Désactive la remontée de mesure par MQTT, lissant uniquement la RF
#define _LORA_P2P_MODE_ // Envoi par LORA
#define CAPTEUR_DHT20
//#define CAPTEUR_DS18B20
#define FLOTTEUR_VERTICAL

//----------------------------------------------------------------------------
// En cas de perte de la configuration, décommenter la ligne suivante
// et remplissez les champs ssid et passwd par vos propres données.
//
// Une fois le Wifi connecté, connectez-vous par Web, reparamétrez les champs
// et revenez ici effacer vos données
//----------------------------------------------------------------------------
//#define FORCE_CONNEXION_WIFI

#ifdef FORCE_CONNEXION_WIFI
#define FORCE_WIFI_SSID  "xxxxxxxx"
#define FORCE_WIFI_PASSWD "xxxxxxx"
#define FORCE_MQTT_SERVER "119.177.244.151"
#define FORCE_MQTT_PORT 1883
#define FORCE_MQTT_USER "ubu_ntu"
#define FORCE_MQTT_PWD "ffffffffff*"
#endif

//#define __ESP32_C3__
//#define __ESP32_S3__
// -----------------------------------------------------------------------------
// Détection automatique de la carte ESP32-C3 ou ESP32-S3
// -----------------------------------------------------------------------------
#if defined(__ESP32_C3__)
  #define IS_ESP32_C3       1
  #define IS_ESP32_S3       0
  #define BOARD_NAME        "ESP32-C3 Super Mini"

  // Pins spécifiques ESP32-C3 (tirés de ton mapping)
  //#define LORA_MISO_PIN     5
  //#define LORA_SCK_PIN      4   // ou 7 selon tes commentaires
  //#define LORA_MOSI_PIN     6
  //#define LORA_CS_PIN       7
  //#define LORA_DIO2_PIN     38  // non utilisé sur C3 ?
  //#define LORA_DIO1_PIN     3   // ou autre libre
  //#define LORA_RESET_PIN    2
  //#define LORA_BUSY_PIN     1   // ou RADIOLIB_NC

    #define DEFAULT_SDA_PIN 8   // Pin 4 // DHT20 entre autre
    #define DEFAULT_SCL_PIN 9   // Pin 5 // DHT20 entre autre

  #ifdef _RCSWITCH_MODE_
  #define CC1101_GDO0       3
  #define CC1101_CS         7
  #define CC1101_MOSI       6
  #define CC1101_MISO       5
  #define CC1101_SCK        4
  #define CC1101_POWER_GND_GPIO 20
  #endif

  #ifdef CAPTEUR_DS18B20
    #define DS18B20_PIN 2 // Pin 111
    // Etat batterie
    #define BATTERIE_PIN    3 // Pin 12
  #else
    // Etat batterie
    #define BATTERIE_PIN    2 // Pin 111
  #endif // CAPTEUR_DS18B20

  #ifdef FLOTTEUR_VERTICAL
    #define DEFAULT_TOR_PIN 1 // Pin 10
  #endif

#elif defined(__ESP32_S3__)
  #define IS_ESP32_C3       0
  #define IS_ESP32_S3       1
  #define BOARD_NAME        "ESP32-S3 (XIAO / Super Mini S3)"

  #ifdef _LORA_P2P_MODE_
    // Pins spécifiques ESP32-S3 (tirés de ton mapping)
    #define LORA_MISO_PIN     8
    #define LORA_SCK_PIN      7
    #define LORA_MOSI_PIN     9
    #define LORA_CS_PIN       41
    #define LORA_DIO2_PIN     38
    #define LORA_DIO1_PIN     39
    #define LORA_RESET_PIN    42
    #define LORA_BUSY_PIN     40

    #define LORA_FREQUENCE    915.0
    #define LORA_BANDWIDTH    125.0
    #define LORA_SPREADING_FACTOR 7
    #define LORA_CODING_RATE  5
    #define LORA_SYNC_WORD    0x12
    #define LORA_OUTPUT_POWER 14.0
    #define LORA_PREAMBLE_LENGTH 8

    #define START_STOP "==========" // A mettre en début et en fin de message
  #endif // _LORA_P2P_MODE_

    #define DEFAULT_SDA_PIN   5   // Pin 5 // DHT20 entre autre
    #define DEFAULT_SCL_PIN   6   // Pin 6 // DHT20 entre autre

  #ifdef CAPTEUR_DS18B20
    #define DS18B20_PIN 2 // Pin 2
    // Etat batterie
    #define BATTERIE_PIN    3 // Pin 3
  #else
    // Etat batterie
    #define BATTERIE_PIN    2 // Pin 2
  #endif // CAPTEUR_DS18B20

  #ifdef FLOTTEUR_VERTICAL
    #define DEFAULT_TOR_PIN 1 // Pin 1
  #endif


#else
  #error "Carte non supportée : ni ESP32-C3 ni ESP32-S3 détectée"
#endif // #if defined(__ESP32_C3__)

#ifdef _RCSWITCH_MODE_
    #define CC1101_ON   HIGH 
    #define CC1101_OFF  LOW 
#endif // _RCSWITCH_MODE_
#ifdef FLOTTEUR_VERTICAL
    #define DEFAULT_TOR_PULL_UP_DOWN_MODE INPUT_PULLUP
#endif

// -----------------------------------------------------------------------------
// Macro pour logs de debug (optionnel)
// -----------------------------------------------------------------------------
#if defined(DEBUG) || defined(_DEBUG)
  #define DEBUG_PRINT(...) Serial.print(__VA_ARGS__)
  #define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__)
#else
  #define DEBUG_PRINT(...)
  #define DEBUG_PRINTLN(...)
#endif

// Espace de nom pour l'enregistrement NVS
#define NVS_NAME_SPACE "ThCh1er"

// Version du logiciel
#define VERSION "1.6"

#endif // __GLOBAL_H__
