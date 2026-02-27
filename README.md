# Home-Assist-ESP32-CYD

Système domotique complet et évolutif basé sur **ESP32** (C3 & S3) avec intégration **LoRa P2P**, **MQTT**, écran tactile **CYD** (Cheap Yellow Display) et capteurs déportés.

## Fonctionnalités principales

- Contrôleur principal avec écran tactile **CYD** (interface locale + MQTT)
- Communication **LoRa P2P** (SX1262) pour longue portée et pénétration (sous-sol, dépendances)
- Ancienne compatibilité **RF 433 MHz** (CC1101)
- Capteurs déportés : température/humidité (**DHT20**), température filaire (**DS18B20**), batterie AA/lithium, tout-ou-rien
- Suivi d'état batterie et alertes faible tension
- Passerelle dédiée **ESP32-S3** (reçoit LoRa ? publie MQTT)
- Mode deep-sleep optimisé sur les capteurs (autonomie plusieurs mois sur piles AA)
- Intégration **Home Assistant** via MQTT

## Matériel utilisé

| Composant                  | Modèle principal                  | Rôle                              |
|----------------------------|------------------------------------|-----------------------------------|
| Contrôleur principal       | ESP32-C3 Super Mini / ESP32-S3     | MQTT + écran CYD                  |
| Écran tactile              | Cheap Yellow Display (ILI9341 + XPT2046) | Interface locale                |
| Capteurs LoRa              | ESP32-C3/S3 + Wio-SX1262           | Température, humidité, batterie   |
| Passerelle LoRa ? MQTT     | ESP32-S3 (XIAO ou autre)           | Relais LoRa ? MQTT                |
| Ancienne RF 433 MHz        | CC1101                             | Compatibilité descendante         |

## Structure du projet
Home-Assist-ESP32-CYD/
+-- Home-Assist/          # Contrôleur principal + CYD (interface MQTT + LoRa gateway)
+-- Capteur/              # Tous les nœuds capteurs LoRa (température, batterie, tout-ou-rien)
+-- Relais-Lora/          # Passerelle ESP32-S3 LoRa ? MQTT
+-- docs/                 # Photos, schémas, captures d'écran (à remplir)
+-- LICENSE               # MIT
+-- README.md             # Ce fichier


Chaque sous-dossier contient son propre `platformio.ini` et peut être ouvert indépendamment dans VS Code + PlatformIO.

## Installation rapide

1. Installer **VS Code** + extension **PlatformIO**
2. Cloner le dépôt :
   ```bash
   git clone https://github.com/jbil-kebir/Home-Assist-ESP32-CYD.git
   
pio run -e esp32c3
pio run -e esp32c3 --target upload
