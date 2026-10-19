# Capteur C3 Xiao — Circuit imprimé

Version du 2026-10-19 (carte V 1.0 dessinée).
**État : schéma, placement, routage, plans de masse, gouttes et sérigraphie FAITS dans EasyEDA Pro ; contrôle des règles
(DRC) = 0 erreur ; schéma et PCB identiques. Prêt pour l'export et la commande** (détail au § 8).

Version **XIAO ESP32-C3** de la carte « Capteur S3 » (`pcb/Capteur S3/`) : même circuit imprimé (46 × 65 mm), mêmes
composants et mêmes variantes, **sans LoRa**. Seules deux liaisons du XIAO changent (flotteur et mesure des piles,
§ 1.1), parce que la broche D0 du C3 est une broche de démarrage.

Méthode et règles : celles de NoeudChauffageSdb et des notices de `D:\Developpement\EasyEda\Notices`.

## 1. Rôle de la carte et code

Deux variantes sur le même circuit imprimé (on ne monte que les composants utiles, § 3) :

| Variante | Mesures | Alimentation |
|---|---|---|
| **Chaudière** | température / humidité (DHT20) + voyant de la chaudière (TCS34725) + LED témoin | prise USB-C du XIAO |
| **Cuve** | température / humidité (DHT20) + niveau haut d'une cuve (flotteur) + tension des piles | 4 piles AA → régulateur MCP1700 |

Remontée par **Wi-Fi / MQTT** (pas de LoRa, pas de DS18B20, pas d'émetteur 433 MHz CC1101 sur cette carte). La carte est
dimensionnée pour un **fonctionnement continu** ; certains capteurs utiliseront le deep sleep.

### 1.1 Brochage du XIAO ESP32-C3

| Fonction | GPIO | Patte XIAO | Constante (`global.h`) | Variante |
|---|---|---|---|---|
| Flotteur (`FLOT`) | 3 | **D1** | `DEFAULT_TOR_PIN` (`FLOTTEUR_VERTICAL`) | cuve |
| Mesure des piles (`VMES`) | 4 | **D2** (ADC1_CH4) | `BATTERIE_PIN` (`CAPTEUR_BATTERIE`) | cuve |
| I2C SDA | 6 | D4 | `DEFAULT_SDA_PIN` | les deux (DHT20, TCS34725) |
| I2C SCL | 7 | D5 | `DEFAULT_SCL_PIN` | les deux |
| LED témoin chaudière | 20 | D7 | `LED_CAPTEUR_RGB_PIN` | chaudière |
| Interruption TCS34725 | 21 | D6 | `CAPTEUR_RGB_TCS34725_INTERRUPT` | non câblée |
| — | 2 | **D0** | — | **non câblée** |
| — | 5, 8, 9, 10 | D3, D8, D9, D10 | — | non câblées |

Pourquoi ce brochage (différent de la carte S3, où le flotteur est sur D0 et la mesure sur D1) :

- **D0 = GPIO2 est une broche de démarrage du C3** : elle doit être à l'état haut au démarrage (y compris au réveil
  d'un deep sleep). Sur D0, le flotteur fermé (cuve pleine) ou le pont de mesure (1,2 à 2 V, ni haut ni bas)
  empêcheraient la carte de démarrer.
- Les entrées analogiques utilisables avec le Wi-Fi (ADC1) ne sortent sur le XIAO C3 que par **D0, D1, D2** → la mesure
  des piles va sur D2.
- **D8 / D9 = GPIO8 / GPIO9** sont aussi des broches de démarrage (GPIO9 = bouton BOOT) : non câblées.

### 1.2 Code

`D:\Developpement\Pilotage_Maison\Capteur`, configuration dans `src/global.h` :

- env PlatformIO **`xiao_esp32c3`** (`board = seeed_xiao_esp32c3`, `-D BOARD_XIAO_C3`), commenté dans `platformio.ini` :
  le décommenter (et commenter les autres envs) pour compiler cette carte ;
- `global.h`, branche C3 : bloc **`BOARD_XIAO_C3`** avec le brochage ci-dessus ; il refuse à la compilation (`#error`)
  les fonctions absentes de cette carte (CC1101, DS18B20) et les combinaisons qui se marchent dessus (micro + flotteur,
  micro + I2C, IR + LED) ; sans `BOARD_XIAO_C3`, la branche C3 reste celle de la Super Mini ;
- `MyWifi.h` : puissance Wi-Fi **19,5 dBm** sur le XIAO C3 (8,5 dBm seulement sur la Super Mini) ;
- plateforme **`espressif32@6.9.0`** (Arduino 2.x / ESP-IDF 4.4) imposée dans `[common]` : le code ne compile pas avec
  pioarduino (Arduino 3 / ESP-IDF 5).

Les broches du flotteur, de la mesure, de l'I2C et de la LED sont aussi réglables par la page Web (NVS).

Schéma de principe des ajouts (identique à la carte S3, sauf les broches du XIAO) :

```
  J3 « PILES »          D2 1N5819        J4 « INTER »         U1 MCP1700-3302E
  4 AA (4,0-6,4 V)                       (interrupteur         (TO-92)
  1 VPIL+ ------------->|-------- VPIL -- 1   en paroi)  2 -- VSW --+-- IN     OUT --+-- 3V3 (XIAO)
  2 GND --+                                                        |     GND        |
          |                                             C3 10 µF --+      |         +-- C4 1 µF
         GND                                                       |     GND        +-- C5 470 µF
                                                                   |
                                          R2 200 k                 |
                               VMES <----/\/\/------------ VSW ----+
                     (D2, GPIO4)  |
                                  +--- R3 100 k --- GND
                                  +--- C6 100 nF -- GND

  J5 « FLOTTEUR »      3V3 --- R4 10 k ---+
  (fils 50 cm)                            |
  1 FLOT_EXT -----------------------------+--- R5 1 k ---+--- FLOT (D1, GPIO3)
  2 GND                                                  |
                                                         +--- C7 100 nF --- GND
     Flotteur normalement ouvert, FERMÉ = CUVE PLEINE
```

## 2. Décisions

| Sujet | Décision |
|---|---|
| Carte | **XIAO ESP32-C3** (antenne externe), sur barrettes femelles ; base = carte Capteur S3 |
| Brochage | flotteur sur **D1**, mesure sur **D2**, **D0 non câblée** (broche de démarrage) |
| LoRa, DS18B20, CC1101 | **aucun** |
| Piles | **4 AA alcalines** (4,0 à 6,4 V), support dans le boîtier imprimé, deux fils vers le bornier J3 ; pas d'accu |
| Régulateur | **MCP1700-3302E/TO**, sortie **directement sur la broche 3V3** du XIAO ; MCP1702-3302E/TO montable |
| Protection | 1N5819 contre l'inversion des piles |
| Interrupteur | **en paroi**, sur fil, relié à J4 |
| Mesure des piles | pont **200 k / 100 k** + 100 nF, pris après l'interrupteur |
| Flotteur | **normalement ouvert, fermé = cuve pleine** ; fils de 50 cm ; rappel 10 k + filtre 1 k / 100 nF |
| LED D1 | **non montée** dans la variante cuve |
| Variantes | décrites **dans ce dossier seulement** |
| Boîtier | impression 3D, non étanche |
| Composants | tout en traversant ; borniers XY301V (piles, flotteur), JST-XH (interrupteur, modules) |

## 3. Composants à monter selon la variante

| Repère | Rôle | Chaudière | Cuve |
|---|---|---|---|
| A1 | XIAO ESP32-C3 | ✔ | ✔ |
| C1, C2 | découplage du 3V3 | ✔ | ✔ |
| TP1–TP4 | points de test 3V3, GND, SDA, SCL | ✔ | ✔ |
| J1 | prise DHT20 | ✔ | ✔ |
| J2 | prise TCS34725 | ✔ | — |
| R1, D1 | LED témoin « CHAUDIERE » | ✔ | — |
| J3, D2, J4 | entrée des piles, protection, interrupteur | — | ✔ |
| C3, U1, C4, C5 | régulateur 3,3 V | — | ✔ |
| R2, R3, C6 | mesure de la tension des piles | — | ✔ |
| J5, R4, R5, C7 | flotteur | — | ✔ |

Variante cuve sans flotteur : ne pas monter J5, R4, R5, C7 (désactiver `FLOTTEUR_VERTICAL`).

## 4. Alimentation et règles d'usage

**Variante chaudière** : bloc USB 5 V sur la prise USB-C du XIAO ; le régulateur du XIAO fournit le 3V3.

**Variante cuve** : 4 AA → D2 → interrupteur → MCP1700 → 3V3 du XIAO.

> **INTERRUPTEUR SUR ARRÊT AVANT DE BRANCHER L'USB** (écrit sur la carte : « USB : INTER sur ARRET »).
> Sinon le régulateur du XIAO et le MCP1700 alimentent le 3V3 en même temps.

Limites du MCP1700 (à surveiller aux essais) :

| Limite | Valeur | Conséquence |
|---|---|---|
| Tension d'entrée | 6,0 V en fonctionnement, 6,5 V maximum absolu | 4 AA neuves ≈ 6,4 V, moins ≈ 0,3 V dans D2 → ≈ 6,0–6,1 V au début de vie des piles seulement |
| Courant de sortie | 250 mA | pointes Wi-Fi ≈ 300–350 mA : **C5 470 µF** fournit l'excédent pendant les pointes |
| Chute de tension | 178 mV à 250 mA | fonctionne jusqu'à ≈ 3,7 V de piles |

Le XIAO C3 émet à 19,5 dBm (pleine puissance) : c'est le cas le plus exigeant pour le MCP1700. Si des redémarrages
apparaissent : C5 à faible résistance interne de même taille (Ø 6,3 mm, pas 2,5), ou baisser la puissance Wi-Fi.

**Autonomie** (indication) : en Wi-Fi continu, 4 AA tiennent **environ un jour** ; l'alimentation par piles suppose le
deep sleep.

## 5. Schéma (EasyEDA « Capteur C3 Xiao », 3 pages)

### 5.1 Page CARTE

| Repère | Composant | Pattes → réseau |
|---|---|---|
| A1 | XIAO ESP32-C3 (fiche perso) | **D1 → `FLOT`** ; **D2 → `VMES`** ; D4 → `SDA` ; D5 → `SCL` ; D7 → `LED_CHAUD` ; 3V3 → `3V3` ; GND → `GND` ; 5V, **D0**, D3, D6, D8–D10 non connectées |
| C1 | céramique 100 nF | `3V3` → `GND` |
| C2 | électrolytique 10 µF 16 V | + → `3V3` |
| R1 | 100 Ω | `LED_CHAUD` → `LED_CHAUD_R` |
| D1 | LED bleue 5 mm | A → `LED_CHAUD_R` ; K → `GND` |

### 5.2 Page PÉRIPHÉRIQUES et 5.3 page ALIMENTATION

Identiques à la carte S3 (`pcb/Capteur S3/CapteurS3.md`, § 5.2 et 5.3) : J1, J2, TP1–TP4, J5, R4, R5, C7 ; J3, D2, J4,
C3, U1, C4, C5, R2, R3, C6.

Bilan : 25 composants, 12 réseaux ; netlist du schéma = PCB (57 liaisons). Par rapport à la carte S3, seules les
liaisons `FLOT` (A1.1 → A1.2) et `VMES` (A1.2 → A1.3) changent.

## 6. Dimensionnement

- **Pont de mesure** : `VMES` = `VSW` / 3 → ≈ 2,0 V au maximum sur D2, dans la zone de mesure du C3 (0–2,5 V environ
  à 11 dB). `VSW` = tension des piles moins la chute de D2 (≈ 0,2–0,3 V).
- **Calibre** à régler : 3,0 (pont 200 k / 100 k) au lieu de 4,3 par défaut — par la page Web (« Calibre »).
- **Flotteur** : rappel de 10 k vers 3V3 ; R5 1 k + C7 100 nF filtrent les parasites du fil. Le code (`INPUT_PULLUP`)
  publie **1 = cuve pleine, 0 = pas pleine**. Sur D1 (GPIO3), l'état du flotteur au démarrage n'a pas d'effet.
- **Largeurs de piste** : `3V3`, `VPIL+`, `VPIL`, `VSW` à 0,6 mm ; signaux à 0,4 mm ; plan `GND` sur les 2 faces.

## 7. Boîtier et mécanique

- Comme la carte S3 : **46 × 65 mm**, 4 trous M3 à 3,5 mm des coins ; J3, J4, J5 au bord haut ; USB-C et LED au bord
  droit ; J1 au bord gauche ; J2 au bord bas.
- **Hauteur** : XIAO C3 seul (sans Wio) sur barrettes femelles → boîtier moins haut que pour la carte S3 (à mesurer :
  hauteur du support de piles et de C5, 11 mm, à prendre en compte).
- **Antenne** : antenne externe du XIAO C3 (connecteur U.FL), à coller sur une paroi, loin du plan de masse.

## 8. Circuit imprimé réalisé (EasyEDA Pro)

### 8.1 Projet

| Élément | Valeur |
|---|---|
| Projet | cloud « Capteur C3 Xiao » (copie par import de `ProPrj_Capteur S3_2026-10-19.epro2`) ; sauvegarde locale : `pcb/Capteur C3 Xiao/ProPrj_Capteur C3 Xiao_2026-10-19.epro2` |
| Schéma | « Capteur C3 Xiao », 3 pages : Carte, Périphériques, Alimentation |
| Règles | « THT soudure main JLCPCB » |

### 8.2 Changements par rapport à la carte S3

- **A1** remplacé par la fiche perso « XIAO ESP32-C3 (THT) » (même symbole, même empreinte « XIAO 14P THT », modèle 3D du
  XIAO C3 tourné de −90°) ; l'import l'avait reposé hors de la carte → remis en (33 ; 18).
- **Pistes** : seul le premier tronçon (diagonale à 45°, face dessous) de `FLOT` et de `VMES` est décalé d'une pastille
  (2,54 mm) pour arriver sur D1 et D2 ; le reste du routage est celui de la carte S3. D0 n'est reliée à rien.
- Plans de masse recoulés ; **DRC = 0 erreur**.
- **Sérigraphie** : titre « Capteur C3 Xiao », « V 1.0 - Octobre 2026 » décalé à droite, « XIAO ESP32-C3 » ; le reste
  est celui de la carte S3. Contrôle : 52 textes, aucun chevauchement, aucun sur une pastille, un via ou un trou.

### 8.3 Reste à faire

- [x] Export `.epro2`, vues 2D / 3D / PCB et PDF dans `pcb/Capteur C3 Xiao/`.
- [x] Commit du dossier de la carte (les modifications de code du § 1.2 ne sont pas commitées).
- [ ] Commande JLCPCB.
- [ ] Essais : démarrage avec cuve pleine, mesure des piles sur D2 (calibre 3,0), Wi-Fi à 19,5 dBm sur piles.
- [ ] Boîtier imprimé.

## 9. Pièces

Identiques à la carte S3 (`pcb/Capteur S3/CapteurS3.md`, § 9), sauf A1 : **XIAO ESP32-C3** (Seeed) au lieu du XIAO
ESP32-S3 + Wio-SX1262.
