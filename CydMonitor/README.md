# CydMonitor

Tableau de bord embarqué sur **ESP32 + écran TFT ILI9341** (CYD — *Cheap Yellow Display*) qui affiche en temps réel l'état d'un luminaire connecté via **MQTT**.

## Fonctionnalités

- Affichage de l'état ON/OFF du luminaire (indicateur visuel coloré)
- Affichage des valeurs RGB et de la luminosité (lux) renvoyées par le capteur de couleur
- Heure courante synchronisée via NTP (fuseau CET/CEST)
- Horodatage du dernier changement d'état
- Indicateur de connexion MQTT
- Interface web de configuration (WiFi, MQTT) accessible sur le port 80
- Mode point d'accès de secours (`CydMonitor`) si la connexion WiFi échoue
- Configuration persistante en flash (NVS)

## Matériel requis

| Composant | Détail |
|-----------|--------|
| Microcontrôleur | ESP32 (dev board standard) |
| Écran | ILI9341 TFT 320×240 (SPI) |
| Rétroéclairage | Pin 21 |
| Touch CS | Pin 33 |

## Affichage (mode paysage 320×240)

```
┌─────────────────────────────────────────┐
│  ThNomade - DEL Monitor      12:34:56   │  ← barre titre (30 px)
├──────────────┬──────────────────────────┤
│              │  R: 128  G: 200  B: 50   │
│   ●  (vert)  │  Lux: 450               │  ← zone principale (175 px)
│              │  Raw: 1024              │
│              │  IP: 192.168.x.x        │
├─────────────────────────────────────────┤
│  Dernier chgt: 12:30:05     [MQTT ✓]   │  ← barre état (35 px)
└─────────────────────────────────────────┘
```

- Cercle **vert** = luminaire allumé
- Cercle **gris foncé** = luminaire éteint

## Protocole MQTT

| Paramètre | Valeur |
|-----------|--------|
| Topic écouté | `home/thermometre/state` |
| Format message | tokens séparés par espaces |
| Message état | `... ThNomade Del on/off ...` |
| Message couleur | `... ThNomade Coul R G B lux raw ...` |

## Configuration

La configuration (WiFi SSID/mot de passe, serveur MQTT/port/identifiants) est modifiable :

1. **Via l'interface web** : connectez-vous à l'IP de l'appareil (ou au point d'accès `CydMonitor` en cas d'échec WiFi), puis ouvrez `http://<ip>/` dans un navigateur.
2. **Via le code source** : les valeurs par défaut se trouvent dans `src/main.cpp` (section `NVS defaults`).

## Installation

### Prérequis

- [PlatformIO](https://platformio.org/) (extension VSCode recommandée)
- Bibliothèques (gérées automatiquement par PlatformIO) :
  - `TFT_eSPI` v2.5.43
  - `PubSubClient` v2.8

### Compilation et flash

```bash
# Compiler
pio run

# Flasher
pio run --target upload

# Moniteur série (115200 baud)
pio device monitor
```

### Paramètres PlatformIO (`platformio.ini`)

Le fichier `platformio.ini` définit le brochage SPI de l'écran ILI9341 et la taille maximale des paquets MQTT (512 octets). Adaptez les pins si votre câblage diffère.

## Structure du projet

```
CydMonitor/
├── platformio.ini      # Configuration PlatformIO (cible, libs, pins TFT)
├── src/
│   └── main.cpp        # Code source principal (451 lignes)
├── include/            # En-têtes (vide)
├── lib/                # Bibliothèques privées (vide)
└── test/               # Tests unitaires (vide)
```

## Lien avec le projet global

CydMonitor s'intègre dans l'écosystème [Home-Assist-ESP32-CYD](../README.md) : il consomme les messages MQTT publiés par le capteur nomade (`Capteur/`) équipé du TCS34725, et offre un affichage dédié indépendant de l'application Flutter.
