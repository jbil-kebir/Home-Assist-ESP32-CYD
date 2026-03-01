# Home-Assist-ESP32-CYD

Syst√®me domotique complet et √©volutif bas√© sur **ESP32** (C3 & S3) avec int√©gration **LoRa P2P**, **MQTT**, √©cran tactile **CYD** (Cheap Yellow Display) et capteurs d√©port√©s.

## Fonctionnalit√©s principales

- Contr√¥leur principal avec √©cran tactile **CYD** (interface locale + MQTT)
- Communication **LoRa P2P** (SX1262) pour longue port√©e et p√©n√©tration (sous-sol, d√©pendances)
- Ancienne compatibilit√© **RF 433 MHz** (CC1101)
- Capteurs d√©port√©s : temp√©rature/humidit√© (**DHT20**), temp√©rature filaire (**DS18B20**), batterie AA/lithium, tout-ou-rien
- Suivi d'√©tat batterie et alertes faible tension
- Passerelle d√©di√©e **ESP32-S3** (re√ßoit LoRa ? publie MQTT)
- Mode deep-sleep optimis√© sur les capteurs (autonomie plusieurs mois sur piles AA)
- Int√©gration **Home Assistant** via MQTT

<<<<<<< HEAD
## AperÁu visuel

### ContrÙleur principal (station maÓtre)

| Photo                                          | Description                                                                 |
|------------------------------------------------|-----------------------------------------------------------------------------|
| ![ContrÙleur principal avec CYD](images/Controleur-primaire-avec-CYD.jpg) | ContrÙleur principal ÈquipÈ de l'Ècran CYD (interface tactile locale)     |
| ![ContrÙleur principal sans CYD](images/Controleur-primaire-sans-CYD.jpg) | MÍme contrÙleur principal, version sans Ècran (montage discret)            |

### ContrÙleur auxiliaire (CYD) ñ Ècrans disponibles

| Photo                                          | Description                                                                 |
|------------------------------------------------|-----------------------------------------------------------------------------|
| ![…cran 1](images/Controleur-Aux-Ecran-1.jpg)  | …cran principal du contrÙleur auxiliaire                                    |
| ![…cran 2](images/Controleur-Aux-Ecran-2.jpg)  | …cran 2 (exemple d'affichage alternatif)                                    |
| ![…cran 3](images/Controleur-Aux-Ecran-3.jpg)  | …cran 3                                                                     |
| ![…cran 4](images/Controleur-Aux-Ecran-4.jpg)  | …cran 4                                                                     |

### Passerelle LoRa ? MQTT

![Passerelle LoRa](images/Relais-Lora.jpg)

Passerelle dÈdiÈe basÈe sur ESP32-S3 qui reÁoit les messages LoRa des capteurs et les publie en MQTT.

### Exemple de capteur LoRa P2P

![Capteur LoRa P2P](images/Capteur-Lora-P2P.jpg)

Núud capteur autonome LoRa P2P avec DHT20 (tempÈrature + humiditÈ) et dÈtecteur de niveau d'eau (non visible sur cette photo).
=======
## Mat√©riel utilis√©

| Composant                  | Mod√®le principal                  | R√¥le                              |
|----------------------------|------------------------------------|-----------------------------------|
| Contr√¥leur principal       | ESP32-C3 Super Mini / ESP32-S3     | MQTT + √©cran CYD                  |
| √âcran tactile              | Cheap Yellow Display (ILI9341 + XPT2046) | Interface locale                |
| Capteurs LoRa              | ESP32-C3/S3 + Wio-SX1262           | Temp√©rature, humidit√©, batterie   |
| Passerelle LoRa ? MQTT     | ESP32-S3 (XIAO ou autre)           | Relais LoRa / MQTT                |
| Ancienne RF 433 MHz        | CC1101                             | Compatibilit√© descendante         |
>>>>>>> 59ba36edb351da530ebde4cb6aaf16d71cfc22a7

## Structure du projet

Home-Assist-ESP32-CYD/
<<<<<<< HEAD
+-- Home-Assist/          # ContrÙleur principal + CYD (interface MQTT + LoRa gateway)
+-- Capteur/              # Tous les núuds capteurs LoRa (tempÈrature, batterie, tout-ou-rien)
+-- Relais-Lora/          # Passerelle ESP32-S3 LoRa ? MQTT
+-- images/               # Photos et visuels
+-- docs/                 # SchÈmas, diagrammes, flux d'information (‡ venir)
=======
+-- Home-Assist/          # Contr√¥leur principal + CYD (interface MQTT + LoRa gateway)
+-- Capteur/              # Tous les n≈ìuds capteurs LoRa (temp√©rature, batterie, tout-ou-rien)
+-- Relais-Lora/          # Passerelle ESP32-S3 LoRa / MQTT
+-- docs/                 # Notices
+-- images/               # Photos, sch√©mas, captures d'√©cran
>>>>>>> 59ba36edb351da530ebde4cb6aaf16d71cfc22a7
+-- LICENSE               # MIT
+-- README.md             # Ce fichier


<<<<<<< HEAD
Chaque sous-dossier est autonome et contient son propre `platformio.ini`.
=======
Chaque sous-dossier contient son propre `platformio.ini` et peut √™tre ouvert ind√©pendamment dans VS Code + PlatformIO.
>>>>>>> 59ba36edb351da530ebde4cb6aaf16d71cfc22a7

## Installation rapide

1. Installer **VS Code** + extension **PlatformIO**
2. Cloner le d√©p√¥t :
   ```bash
   git clone https://github.com/jbil-kebir/Home-Assist-ESP32-CYD.git
   
3. Ouvrir un des sous-dossiers dans VS Code (ex: Home-Assist)
4. SÈlectionner líenvironnement :
    esp32c3 ? ESP32-C3 Super Mini
    esp32s3 ? ESP32-S3 (XIAO ou autre)

5. Compiler / flasher :

pio run -e esp32c3
pio run -e esp32c3 --target upload

## Licence

MIT License ñ voir le fichier [LICENSE](LICENSE)

## Mots-clÈs

Domotique, CYD, ESP32, ESP32-C3, ESP32-S3, LoRa, LoRa P2P, SX1262, MQTT, Home Assistant, DHT20, DS18B20, Batterie, Capteur, Passerelle, RF 433 MHz, CC1101
