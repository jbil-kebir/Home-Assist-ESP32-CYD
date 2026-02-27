#ifndef __GLOBAL_H__
#define __GLOBAL_H__


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

#define LORA_FREQUENCE          915.0 // Europe 868 915.0
#define LORA_BANDWIDTH          125.0
#define LORA_SPREADING_FACTOR    7 // 12 SF12 = max portée
#define LORA_CODING_RATE         5 // 4 ou 5
#define LORA_SYNC_WORD          0x12
#define LORA_OUTPUT_POWER       14.0    //17
#define LORA_PREAMBLE_LENGTH    8   // 10

#define START_STOP "==========" // A retirer du début et de la fin des messages reçus avant réexpédition par MQTT


// Espace de nom pour l'enregistrement NVS
#define NVS_NAME_SPACE "RelaisLora"


// Version du logiciel
#define VERSION "1.0"


#endif // __GLOBAL_H__
