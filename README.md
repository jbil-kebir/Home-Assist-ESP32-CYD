# Pilotage Maison ESP32

Système domotique complet basé sur ESP32 (C3 & S3) avec :

- Communication **LoRa P2P** (SX1262)
- Capteurs température/humidité/batterie/tout-ou-rien
- Écran tactile **CYD** (Cheap Yellow Display)
- Passerelle LoRa ? MQTT
- Intégration **Home Assistant** via MQTT

## Structure du projet

- **Home-Assist** : contrôleur principal + interface CYD
- **Capteur**     : nœuds capteurs LoRa (température, batterie, etc.)
- **Relais-Lora** : passerelle S3 (reçoit LoRa ? publie MQTT)

## Matériel principal

- ESP32-C3 Super Mini / ESP32-S3 XIAO
- Wio-SX1262 LoRa
- DHT20 / DS18B20
- CC1101 (ancienne version 433 MHz)
- Écran ILI9341 + XPT2046

## Installation rapide

1. Installer **PlatformIO** (VS Code extension)
2. Ouvrir un des dossiers (Home-Assist, Capteur, Relais-Lora)
3. `pio run -e esp32c3` ou `pio run -e esp32s3` selon la carte
4. Flasher via USB

Plus de détails dans chaque sous-dossier.

Licence : MIT
