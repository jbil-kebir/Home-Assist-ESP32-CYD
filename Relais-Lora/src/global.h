#ifndef __GLOBAL_H__
#define __GLOBAL_H__

//------------------------------------------------------------------------------
// Journal web (http://<IP>/logs) : Serial est redirigé vers gSerialJournal, qui écrit
// sur le port série ET dans le journal (voir MyJournal.h, copie de Capteur/src/MyJournal.*).
// Arduino.h doit être inclus avant le #define (il déclare le vrai Serial, parfois sous forme de macro
// quand ARDUINO_USB_CDC_ON_BOOT=1 : d'où le #undef).
//------------------------------------------------------------------------------
#include <Arduino.h>
#include "MyJournal.h"
#ifdef Serial
#undef Serial
#endif
#define Serial gSerialJournal


//----------------------------------------------------------------------------
// A décommenter au premier démarrage.
// Remplissez les champs ssid et passwd par vos propres données.
// 
// Les accès MQTT seront désactivés car les topics ne sont pas encore 
// valides. Il faudra accéder à la page Web pour en saisir des valides.
//----------------------------------------------------------------------------
//#define __PREMIER_DEMARRAGE__
//----------------------------------------------------------------------------
// En cas de perte de la configuration, décommenter la ligne suivante
// et remplissez les champs ssid et passwd par vos propres données.
//
// Une fois le Wifi connecté, connectez-vous par Web, reparamétrez les champs
// et revenez ici effacer vos données
//----------------------------------------------------------------------------
//#define __FORCE_CONNEXION_WIFI__

#if defined(__FORCE_CONNEXION_WIFI__) || defined(__PREMIER_DEMARRAGE__)
#define FORCE_WIFI_SSID  "xxxxxxxx"
#define FORCE_WIFI_PASSWD "xxxxxxx"
#define FORCE_MQTT_SERVER "119.177.244.151"
#define FORCE_MQTT_PORT 1883
#define FORCE_MQTT_USER "ubu_ntu"
#define FORCE_MQTT_PWD "ffffffffff*"
#endif


// Config Lora
#define LORA_MISO_PIN           8   
#define LORA_SCK_PIN            7    
#define LORA_MOSI_PIN           9  
#define LORA_CS_PIN             41    
#define LORA_DIO2_PIN           38
#define LORA_DIO1_PIN           39     
#define LORA_RESET_PIN          42
#define LORA_BUSY_PIN           40

#define LORA_FREQUENCE          868.1 // Europe : sous-bande g1 868,0-868,6 MHz (1 % du temps d'émission, 14 dBm)
#define LORA_BANDWIDTH          125.0
#define LORA_SPREADING_FACTOR    7 // 12 SF12 = max portée
#define LORA_CODING_RATE         5 // 4 ou 5
#define LORA_SYNC_WORD          0x12
#define LORA_OUTPUT_POWER       14.0    //17
#define LORA_PREAMBLE_LENGTH    8   // 10
#define LORA_TCXO_VOLTAGE       1.8 // Wio-SX1262 Seeed : TCXO 1.8V (et non 1.6V)

#define START_STOP "==========" // A retirer du début et de la fin des messages reçus avant réexpédition par MQTT

// File d'attente des commandes de config vers les capteurs LoRa (FileCommandes.cpp)
// { topic de config, suffixe des noms d'équipement du capteur } : un par capteur LoRa
// (l'ajouter aussi à la liste NOEUDS de HA/genere_noeuds.py, qui génère le paquet et la vue HA)
#define FILE_CMD_CAPTEURS { \
  { "home/confthremise/command", "Remise" }, \
  { "home/confthnomade/command", "Nomade" }, \
  { "home/confthcave/command", "Cave" } }
#define FILE_CMD_NB_MAX_CAPTEURS    8
// Silence après la dernière trame du capteur avant d'envoyer : plus long qu'une trame de 255 octets (~390 ms en SF7 /
// 125 kHz), sinon le relais émet par-dessus la trame suivante du capteur (VERSION, CONFIG perdues : 16/11, avec 300 ms).
// Le capteur écoute LORA_FENETRE_ECOUTE_MS = 1,5 s après sa dernière émission (Capteur 4.0 ; 1 s avant)
#define FILE_CMD_SILENCE_MS         600
#define FILE_CMD_NOUVEAU_REVEIL_MS  5000  // Silence au-delà duquel une trame marque un nouveau réveil
#define FILE_CMD_MAX_ENVOIS         10    // Réveils sans accusé avant abandon (capteur éveillé : envois sans accusé)
// Capteur ÉVEILLÉ (dernière config reçue : sleepA:0, et dernière trame autre que DEEPSLEEP ON) : il écoute en
// permanence, mais n'émet que sur changement de mesure ou au forçage (jusqu'à 1 h) ; attendre une de ses trames
// retardait d'autant la commande (ThRemise, 21/11/2026). Commande envoyée directement, puis renvoyée toutes les
// FILE_CMD_RENVOI_EVEILLE_MS jusqu'à l'accusé (FILE_CMD_MAX_ENVOIS envois au plus, puis abandon)
#define FILE_CMD_RENVOI_EVEILLE_MS  30000


// Espace de nom pour l'enregistrement NVS
#define NVS_NAME_SPACE "RelaisLora"


// Version du logiciel
#define VERSION "3.1"
// Page Web (3.0) : signe ajouté au libellé des réglages qui ne s'appliquent pas à chaud. À tenir en accord avec
// empreintesReglages (MyWebServer.cpp), qui décide réellement du redémarrage ou de la reconnexion MQTT
#define MARQUE_REDEMARRAGE "<span class=\"mq-r\" title=\"Modification appliquée après un redémarrage\"> ↻</span>"
#define MARQUE_MQTT "<span class=\"mq-m\" title=\"Modification appliquée par une reconnexion MQTT, sans redémarrage\"> ⇄</span>"
#define LEGENDE_MARQUES "<span class=\"mq-r\">↻</span> redémarrage · <span class=\"mq-m\">⇄</span> reconnexion MQTT · sans signe : immédiat"
// Étiquette du firmware (MyOta.cpp) : la page /update refuse un fichier qui n'a pas celle de l'appareil
#define ETIQUETTE_FIRMWARE "Relais-Lora"
// Attente maximale de la fin d'une émission LoRa avant d'en commencer une autre (CMyLoraRxTx::sendPacket())
#define LORA_ATTENTE_TX_MS 2000
// Écart minimal entre la fin d'une émission et le début de la suivante (CMyLoraRxTx::pretAEmettre()) : le capteur
// qui vient de recevoir un paquet le traite avant de se remettre en écoute, et rate le préambule d'un paquet émis
// juste derrière. Le 21/11/2026, SET sleepA:0 émis 4 ms après l'ACK du flotteur : jamais reçu par ThRemise (7 essais).
// Reste bien inférieur à la fenêtre d'écoute du capteur (LORA_FENETRE_ECOUTE_MS = 1,5 s, Capteur 4.0)
#define LORA_ECART_EMISSIONS_MS 300
// Envois différés (ACK du flotteur relayé depuis MQTT) : émis après FILE_CMD_SILENCE_MS sans trame reçue,
// au plus LORA_DIFFERE_MAX_MS après leur mise en attente ; LORA_DIFFERES_MAX messages en attente au plus
#define LORA_DIFFERE_MAX_MS 5000
#define LORA_DIFFERES_MAX   4


#endif // __GLOBAL_H__
