# Capteur S3 — Circuit imprimé

Version du 2026-10-19 (carte V 1.0 dessinée).
**État : schéma, placement, routage, plans de masse, gouttes et sérigraphie FAITS dans EasyEDA Pro ; contrôle des règles
(DRC) = 0 erreur ; schéma et PCB identiques. Prêt pour l'export et la commande** (détail au § 8).

Carte commune à plusieurs capteurs à base de **XIAO ESP32-S3** (avec ou sans Wio-SX1262). Elle reprend la carte
« Capteur chaudière Sdb » (`pcb/Capteur chaudiere Sdb/`), qui sera supprimée à terme, et lui ajoute une alimentation
par piles, la mesure de leur tension et une entrée tout ou rien (flotteur). Une carte « Capteur C3 Xiao » suivra.

Méthode et règles : celles de NoeudChauffageSdb et des notices de `D:\Developpement\EasyEda\Notices` (modèle de règles
PCB, routage, bibliothèque personnelle).

## 1. Rôle de la carte et code

Deux variantes sur le même circuit imprimé (on ne monte que les composants utiles, § 3) :

| Variante | Mesures | Alimentation |
|---|---|---|
| **Chaudière** | température / humidité (DHT20) + voyant de la chaudière (TCS34725) + LED témoin | prise USB-C du XIAO |
| **Cuve** | température / humidité (DHT20) + niveau haut d'une cuve (flotteur) + tension des piles | 4 piles AA → régulateur MCP1700 |

Remontée par **Wi-Fi / MQTT** ; LoRa (Wio-SX1262) activable (`_LORA_P2P_MODE_`). La carte est dimensionnée pour un
**fonctionnement continu** : certains capteurs utiliseront le deep sleep, d'autres non.

Code : `D:\Developpement\Pilotage_Maison\Capteur`, env PlatformIO `esp32s3`, configuration dans `src/global.h`.

| Fonction | GPIO | Patte XIAO | Constante (`global.h`) | Variante |
|---|---|---|---|---|
| Flotteur (`FLOT`) | 1 | D0 | `DEFAULT_TOR_PIN` (`FLOTTEUR_VERTICAL`) | cuve |
| Mesure des piles (`VMES`) | 2 | D1 | `BATTERIE_PIN` (`CAPTEUR_BATTERIE`) | cuve |
| I2C SDA | 5 | D4 | `DEFAULT_SDA_PIN` | les deux (DHT20, TCS34725) |
| I2C SCL | 6 | D5 | `DEFAULT_SCL_PIN` | les deux |
| LED témoin chaudière | 44 | D7 | `LED_CAPTEUR_RGB_PIN` | chaudière |
| Interruption TCS34725 | 43 | D6 | `CAPTEUR_RGB_TCS34725_INTERRUPT` | non câblée |
| SPI du Wio-SX1262 | 7, 8, 9 | D8, D9, D10 | `LORA_*_PIN` | laissées libres |

Les broches du flotteur, de la mesure, de l'I2C et de la LED sont aussi réglables par la page Web (NVS).

Schéma de principe des ajouts (le reste est celui de la carte chaudière) :

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
                     (D1, GPIO2)  |
                                  +--- R3 100 k --- GND
                                  +--- C6 100 nF -- GND

  J5 « FLOTTEUR »      3V3 --- R4 10 k ---+
  (fils 50 cm)                            |
  1 FLOT_EXT -----------------------------+--- R5 1 k ---+--- FLOT (D0, GPIO1)
  2 GND                                                  |
                                                         +--- C7 100 nF --- GND
     Flotteur normalement ouvert, FERMÉ = CUVE PLEINE
```

### 1.1 Schémas de câblage par variante

Un dessin complet par variante, d'après le schéma EasyEDA (`Capteur_S3_Projet_2026-10-19.epro2`). Le XIAO est vu de dessus, USB-C en haut ;
1 à 14 = numéros de ses pattes.

Conventions : `+` = connexion ; un trait qui en coupe un autre sans `+` n'y est pas relié ; toutes les masses `GND`
sont reliées entre elles ; les broches des modules portent le numéro de leur prise (J1, J2, J5).

Repères communs aux variantes sur piles :

- **D2 1N5819** : bague (cathode) côté interrupteur.
- **U1 MCP1700 (TO-92)** : face plate vers soi, pattes en bas : 1 GND, 2 IN, 3 OUT.
- **Électrolytiques** : `+` côté VSW pour C3, côté 3V3 pour C2 et C5.
- **DHT20** : broches du module **+ − SCL SDA** = J1 1 à 4 ; sur le dessin, elles se lisent de bas en haut.
- **Pont R2 / R3** pris sur VSW, après l'interrupteur ; calibre 3,0.
- **5V** du XIAO jamais relié aux piles ; **interrupteur sur arrêt avant de brancher l'USB**.
- **Wio-SX1262** : monté au dos du XIAO (connecteur), non dessiné ; il utilise D8, D9, D10 (SPI) : ne rien y câbler.

> **IMPORTANT — broches enregistrées en NVS.** Les broches (I2C, mesure des piles, flotteur, LED) sont enregistrées avec
> la configuration par la page Web ; au démarrage, le code relit ces valeurs et n'utilise celles de `global.h` que si
> rien n'est enregistré. Une configuration enregistrée avec un autre firmware garde donc ses broches même après le
> passage au firmware `esp32s3`. **Après chaque téléversement, vérifier sur la page Web** :
>
> | Réglage | Valeur (GPIO) | Patte XIAO |
> |---|---|---|
> | SDA (DHT20, TCS34725) | 5 | D4 |
> | SCL (DHT20, TCS34725) | 6 | D5 |
> | Mesure des piles (+ calibre 3,0) | 2 | D1 |
> | Flotteur | 1 | D0 |
> | LED témoin chaudière | 44 | D7 |

**Placement des condensateurs** (montage d'essai ou carte) : chacun au plus près des pattes qu'il relie, fils courts ;
toutes les masses au même GND.

| Repère | Type | Entre | Où le placer | Rôle |
|---|---|---|---|---|
| C1 | céramique 100 nF | 3V3 et GND | contre les pattes 12 (3V3) et 13 (GND) du XIAO | découplage du XIAO |
| C2 | électrolytique 10 µF, **+ côté 3V3** | 3V3 et GND | près du XIAO, à côté de C1 | réserve locale du 3V3 |
| C3 | électrolytique 10 µF, **+ côté VSW** | VSW et GND | contre les pattes 2 (IN) et 1 (GND) du MCP1700 | stabilité de l'entrée du régulateur |
| C4 | céramique 1 µF | 3V3 et GND | contre les pattes 3 (OUT) et 1 (GND) du MCP1700, à moins de 1 cm | **obligatoire** : sans lui, le MCP1700 peut osciller |
| C5 | électrolytique 470 µF, **+ côté 3V3** | 3V3 et GND | sur le 3V3 entre le MCP1700 et le XIAO, fils courts et gros | fournit les pointes Wi-Fi / LoRa au-delà des 250 mA du MCP1700 |
| C6 | céramique 100 nF | VMES et GND | contre la patte de mesure du XIAO (D1) | filtre la mesure des piles |
| C7 | céramique 100 nF | FLOT et GND | contre la patte du flotteur (D0), après R5 | filtre les parasites du fil du flotteur |

**Montage d'essai sans 1N5819 (D2)** : la remplacer par un fil (+ des piles → interrupteur). Conséquences :

- **plus de protection contre l'inversion des piles** (MCP1700 et XIAO détruits) : vérifier la polarité au multimètre
  avant la mise en marche ;
- **entrée du MCP1700** : 4 AA neuves ≈ 6,4 V, au-dessus des 6,0 V de fonctionnement (maximum absolu 6,5 V). Si le bloc
  dépasse 6,0 V : piles un peu entamées, ou 3 AA (4,5 V) provisoirement ;
- **mesure des piles** : VSW = tension réelle des piles, soit 0,2 à 0,3 V **de plus** qu'avec la diode → ne pas régler le
  seuil « piles déchargées » sur ce montage ;
- **consommation en deep sleep** : inchangée (la diode ne consomme rien). Le pont R2 / R3 consomme, lui, ≈ 20 µA en
  permanence, interrupteur en marche ;
- dépannage possible avec une 1N4001…1N4007 : protection conservée, mais chute de 0,7 à 0,9 V (mesure d'autant plus basse).

#### a) Piles + DHT20 (cuve sans flotteur)

```
   D2 1N5819   J4 inter                   VSW
  +---->|-------o/ o----+---------+-----------------+
  |                     |         | 2 IN            |             XIAO ESP32-S3 (vu de dessus)
 (+)                C3 10 µF +---------+        R2 200 k              +---[ USB-C ]---+
PILES                   |    | U1      |-(1)-- GND  |           D0  1 | o           o | 14  5V
4 AA                   GND   | MCP1700 |      VMES  +---------- D1  2 | o           o | 13  GND --- GND
 (-)                         | 3302E   |            |           D2  3 | o           o | 12  3V3 --------+
  |                          +---------+  +---------+           D3  4 | o           o | 11  D10         |
 GND                        3 OUT |       |         |    +----- D4  5 | o           o | 10  D9          |
                                  |   C6 100 nF R3 100 k |   +- D5  6 | o           o |  9  D8          |
                                  |       |         |    |   |  D6  7 | o           o |  8  D7          |
                                  |      GND       GND   |   |        +---------------+                 |
                                  |                      |   |                                          |
                                  |                      |   |                                          |
                                  |       +-----------+  |   |                                          |
                                  |       |DHT20 4 SDA|--+   |                                          |
                                  |       |      3 SCL|------+                                          |
                                  |       | J1   2 -  |--- GND                                          |
                                  |       |      1 +  |--+       GND        GND        GND        GND   |
                                  |       +-----------+  |        |          |          |          |    |
                                  |                      |     C4 1 µF   C5 470 µF  C1 100 nF  C2 10 µF |
                                  |                      |        |          |          |          |    |
                                  +----------------------+--------+----------+----------+----------+----+
                                                               3V3
```

Pattes libres : D0, D2, D3, D6, D7, 5V ; D8, D9, D10 réservées au Wio-SX1262.

#### b) Cuve : piles + DHT20 + flotteur

```
                                              +--------------------------- 3V3 -------------------------+
                                              |                                                         |
                                           R4 10 k                                                      |
                            +--------+FLOT_EXT|                                                         |
                            | J5   1 |--------+- R5 1 k +---+                                           |
                            |      2 |-- GND            |   |                                           |
                            +--------+              C7 100 nF                                           |
   D2 1N5819   J4 inter    vers flotteur  VSW           |   |                                           |
  +---->|-------o/ o----+---------+-----------------+  GND  |                                           |
  |                     |         | 2 IN            |       |     XIAO ESP32-S3 (vu de dessus)          |
 (+)                C3 10 µF +---------+        R2 200 k    |         +---[ USB-C ]---+                 |
PILES                   |    | U1      |-(1)-- GND  |       +-- D0  1 | o           o | 14  5V          |
4 AA                   GND   | MCP1700 |      VMES  +---------- D1  2 | o           o | 13  GND --- GND |
 (-)                         | 3302E   |            |           D2  3 | o           o | 12  3V3 --------+
  |                          +---------+  +---------+           D3  4 | o           o | 11  D10         |
 GND                        3 OUT |       |         |    +----- D4  5 | o           o | 10  D9          |
                                  |   C6 100 nF R3 100 k |   +- D5  6 | o           o |  9  D8          |
                                  |       |         |    |   |  D6  7 | o           o |  8  D7          |
                                  |      GND       GND   |   |        +---------------+                 |
                                  |                      |   |                                          |
                                  |                      |   |                                          |
                                  |       +-----------+  |   |                                          |
                                  |       |DHT20 4 SDA|--+   |                                          |
                                  |       |      3 SCL|------+                                          |
                                  |       | J1   2 -  |--- GND                                          |
                                  |       |      1 +  |--+       GND        GND        GND        GND   |
                                  |       +-----------+  |        |          |          |          |    |
                                  |                      |     C4 1 µF   C5 470 µF  C1 100 nF  C2 10 µF |
                                  |                      |        |          |          |          |    |
                                  +----------------------+--------+----------+----------+----------+----+
                                                               3V3
```

- **J5 flotteur** : normalement ouvert, fermé = cuve pleine ; fils de 50 cm.

Pattes libres : D2, D3, D6, D7, 5V ; D8, D9, D10 réservées au Wio-SX1262.

#### c) Chaudière : USB + DHT20 + TCS34725 + LED

```
                                                                  alimentation : câble USB-C 5 V
                                                                  XIAO ESP32-S3 (vu de dessus)
                                                                      +---[ USB-C ]---+
                                                                D0  1 | o           o | 14  5V
                                                                D1  2 | o           o | 13  GND ---- GND
                                                                D2  3 | o           o | 12  3V3 ---- 3V3
                                                                D3  4 | o           o | 11  D10
                                                         +----- D4  5 | o           o | 10  D9
                                                         |   +- D5  6 | o           o |  9  D8
                                                         |   |  D6  7 | o           o |  8  D7  ----+
                                                         |   |        +---------------+             |
                                    +-----------+        |   |                                  R1 100 Ω
                                    |DHT20 4 SDA|--------+   |                                      |
                                    |      3 SCL|--------|---+                                      V   D1 LED bleue
                                    | J1   2 -  |-- GND  |   |             3V3         3V3          -   (méplat en bas)
                                    |      1 +  |-- 3V3  |   |              |           |           |
                                    +-----------+        |   |          C1 100 nF   C2 10 µF       GND
                                                         |   |              |           |
                                    +-----------+        |   |             GND         GND
                                    |TCS   5 LED|-- GND  |   |
                                    |34725 4 SDA|--------+   |
                                    | J2   3 SCL|------------+
                                    |      2 -  |-- GND
                                    |      1 +  |-- 3V3
                                    +-----------+
```

- Alimentation par le câble USB-C (régulateur du XIAO) ; J3, D2, J4, U1, C3 à C7, R2 à R5 et J5 non montés.
- Un seul croisement : la ligne SCL du DHT20 coupe la ligne SDA sans y être reliée.
- **J2** : 1 = + (VIN du module), 2 = −, 3 = SCL, 4 = SDA, 5 = LED du module reliée à GND (LED éteinte) ; INT du module
  non câblée.
- **D1 LED bleue** : patte longue (anode) côté R1, méplat / patte courte côté GND.

Pattes libres : D0, D1, D2, D3, D6 ; D8, D9, D10 réservées au Wio-SX1262 ; 5V = entrée USB, rien à y câbler.

Points de test TP1 à TP4 (3V3, GND, SDA, SCL) non dessinés.

## 2. Décisions

| Sujet | Décision |
|---|---|
| Carte | XIAO ESP32-S3 (+ Wio-SX1262 si LoRa), sur barrettes femelles ; base = carte chaudière |
| Piles | **4 AA alcalines** (4,0 à 6,4 V), support dans le boîtier imprimé, deux fils vers le bornier J3 ; pas d'accu |
| Régulateur | **MCP1700-3302E/TO** (stock de l'utilisateur), sortie **directement sur la broche 3V3** du XIAO ; un MCP1702-3302E/TO (même brochage, entrée 13,2 V) se monte aussi |
| Protection | 1N5819 contre l'inversion des piles |
| Interrupteur | **en paroi**, sur fil, relié à J4 (coupe franche des piles) |
| Mesure des piles | pont **200 k / 100 k** + 100 nF, pris **après l'interrupteur** (aucune consommation à l'arrêt) |
| Flotteur | **normalement ouvert, fermé = cuve pleine** ; fils de 50 cm sur bornier ; mesure toutes les 2 à 5 min ; rappel 10 k + filtre 1 k / 100 nF |
| LED D1 | **non montée** dans la variante cuve |
| Variantes | décrites **dans ce dossier seulement** (rien sur la sérigraphie) |
| Boîtier | impression 3D, non étanche |
| Composants | tout en traversant ; borniers à vis XY301V pour les piles et le flotteur, JST-XH pour l'interrupteur et les modules |

## 3. Composants à monter selon la variante

| Repère | Rôle | Chaudière | Cuve |
|---|---|---|---|
| A1 | XIAO ESP32-S3 (+ Wio-SX1262) | ✔ | ✔ |
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

**Variante cuve** : 4 AA → D2 → interrupteur → MCP1700 → 3V3 du XIAO (le régulateur du XIAO n'est pas utilisé).

> **INTERRUPTEUR SUR ARRÊT AVANT DE BRANCHER L'USB** (écrit sur la carte : « USB : INTER sur ARRET »).
> Sinon le régulateur du XIAO et le MCP1700 alimentent le 3V3 en même temps. Pour programmer la carte : interrupteur
> sur arrêt, puis câble USB.

Limites du MCP1700 (à surveiller aux essais) :

| Limite | Valeur | Conséquence |
|---|---|---|
| Tension d'entrée | 6,0 V en fonctionnement, 6,5 V maximum absolu | 4 AA neuves ≈ 6,4 V, moins ≈ 0,3 V dans D2 → ≈ 6,0–6,1 V au début de vie des piles seulement |
| Courant de sortie | 250 mA | pointes Wi-Fi ≈ 300–350 mA (et ≈ 120 mA de plus si le LoRa émet) : **C5 470 µF** fournit l'excédent pendant les pointes |
| Chute de tension | 178 mV à 250 mA | fonctionne jusqu'à ≈ 3,7 V de piles (4 × 1,0 V moins D2) |

Si des redémarrages apparaissent en Wi-Fi continu : C5 à faible résistance interne de même taille (Ø 6,3 mm, pas 2,5,
par exemple série Rubycon ZLH), puis, si besoin, un régulateur plus puissant dans une version suivante.

**Autonomie** (indication) : en Wi-Fi continu (≈ 100 mA en moyenne), 4 AA (≈ 2 500 mAh) tiennent **environ un jour** ;
l'alimentation par piles suppose donc le deep sleep (réveil toutes les quelques minutes).

## 5. Schéma (EasyEDA « Capteur S3 », 3 pages)

### 5.1 Page CARTE (inchangée, sauf D0 et D1 du XIAO)

| Repère | Composant | Pattes → réseau |
|---|---|---|
| A1 | XIAO ESP32-S3 + Wio-SX1262 (fiche perso) | D0 → `FLOT` ; D1 → `VMES` ; D4 → `SDA` ; D5 → `SCL` ; D7 → `LED_CHAUD` ; 3V3 → `3V3` ; GND → `GND` ; 5V, D2, D3, D6, D8–D10 non connectées |
| C1 | céramique 100 nF | `3V3` → `GND` |
| C2 | électrolytique 10 µF 16 V | + → `3V3` |
| R1 | 100 Ω | `LED_CHAUD` → `LED_CHAUD_R` |
| D1 | LED bleue 5 mm | A → `LED_CHAUD_R` ; K → `GND` |

### 5.2 Page PÉRIPHÉRIQUES

| Repère | Composant | Pattes → réseau |
|---|---|---|
| J1 | JST-XH 4 « vers DHT20 » | 1 `3V3`, 2 `GND`, 3 `SCL`, 4 `SDA` |
| J2 | JST-XH 5 « vers TCS34725 » | 1 `3V3` (VIN du module), 2 `GND`, 3 `SCL`, 4 `SDA`, 5 `GND` (LED du module éteinte) |
| TP1–TP4 | points de test | `3V3`, `GND`, `SDA`, `SCL` |
| J5 | bornier XY301V 2P « vers flotteur (50 cm) » | 1 `FLOT_EXT`, 2 `GND` |
| R4 | 10 kΩ, rappel | `3V3` → `FLOT_EXT` |
| R5 | 1 kΩ, filtre | `FLOT_EXT` → `FLOT` |
| C7 | céramique 100 nF, filtre | `FLOT` → `GND` |

### 5.3 Page ALIMENTATION (nouvelle)

| Repère | Composant | Pattes → réseau |
|---|---|---|
| J3 | bornier XY301V 2P « vers piles 4 AA » | 1 `VPIL+`, 2 `GND` |
| D2 | 1N5819 | A → `VPIL+` ; K → `VPIL` |
| J4 | JST-XH 2P « vers interrupteur M/A » | 1 `VPIL`, 2 `VSW` |
| C3 | électrolytique 10 µF 16 V | + → `VSW` |
| U1 | MCP1700-3302E (TO-92) | 1 GND → `GND` ; 2 IN → `VSW` ; 3 OUT → `3V3` |
| C4 | céramique 1 µF 50 V (obligatoire pour la stabilité du MCP1700) | `3V3` → `GND` |
| C5 | électrolytique 470 µF 10 V (réserve pour les pointes Wi-Fi) | + → `3V3` |
| R2 | 200 kΩ | `VSW` → `VMES` |
| R3 | 100 kΩ | `VMES` → `GND` |
| C6 | céramique 100 nF | `VMES` → `GND` |

Bilan : 25 composants, 12 réseaux ; netlist du schéma = PCB (57 liaisons).

## 6. Dimensionnement

- **Pont de mesure** : `VMES` = `VSW` / 3. Piles neuves 6,4 V → `VSW` ≈ 6,1 V → 2,0 V sur D1, dans la zone linéaire du
  convertisseur du S3 (0–3,3 V à 11 dB). Consommation du pont : ≈ 20 µA, seulement interrupteur en marche.
  `VSW` = tension des piles moins la chute de D2 (≈ 0,2–0,3 V, variable avec le courant) : en tenir compte dans le seuil
  « piles déchargées » ou corriger le calibre.
- **Calibre** à régler : 3,0 (pont 200 k / 100 k) au lieu de 4,3 par défaut dans le code (pont 330 k / 100 k) — par la
  page Web (« Calibre ») ou en changeant `mDefaultCalibre` dans `MyBatterieAA.h`.
- **Flotteur** : rappel de 10 k vers 3V3 (le rappel interne du S3, ≈ 45 k, est faible pour 50 cm de fil) ; R5 1 k + C7
  100 nF filtrent les parasites captés par le fil (constante de temps 0,1 ms, sans effet sur une mesure toutes les
  minutes). Contact fermé (cuve pleine) : 0,33 mA dans R4. Le code (`INPUT_PULLUP`) inverse la lecture : il publie
  **1 = cuve pleine, 0 = pas pleine**.
- **Largeurs de piste** : `3V3`, `VPIL+`, `VPIL`, `VSW` à 0,6 mm ; signaux à 0,4 mm ; plan `GND` sur les 2 faces.

## 7. Boîtier et mécanique

- **Impression 3D**, non étanche ; support 4 AA dans le boîtier (variante cuve).
- **Carte 46 × 65 mm**, 4 trous M3 à 3,5 mm des coins.
- **Bord haut** : J3 (piles), J4 (interrupteur), J5 (flotteur), entrée des fils vers le bord → passages de fils dans la
  paroi de ce côté ; interrupteur en paroi.
- **Bord droit** : USB-C du XIAO et LED D1 (variante chaudière) ; **bord gauche** : J1 (DHT20) ; **bord bas** : J2 (TCS34725).
- **Hauteur** : XIAO + Wio sur barrettes femelles → hauteur intérieure ≥ 25 mm (comme la carte chaudière).

## 8. Circuit imprimé réalisé (EasyEDA Pro)

### 8.1 Projet

| Élément | Valeur |
|---|---|
| Projet | cloud « Capteur S3 » (copie par import de `CapteurChaudiereSdb.epro2`) ; sauvegarde locale : `pcb/Capteur S3/Capteur_S3_Projet_2026-10-19.epro2` |
| Schéma | « Capteur S3 », 3 pages : Carte, Périphériques, Alimentation |
| Règles | « THT soudure main JLCPCB » (modèle par défaut) |
| Classes | ALIM_3V3 (`3V3`, `GND`) 0,6 mm ; `VPIL+`, `VPIL`, `VSW` 0,6 mm ; signaux 0,4 mm |

### 8.2 Carte et placement

- **46 × 65 mm**, 2 couches ; la partie basse (46 × 41) est la carte chaudière, inchangée.
- Bande haute, de haut en bas : J3 / J4 (tourné de 180°) / J5 au bord ; D2, C3, U1, C4, C5 (dans l'ordre du courant) ;
  R2, R3, C6 (pont) ; R4 et R5 debout (rotation 90°) et C7 à droite, près des broches D0 / D1 du XIAO.
- Contrôle d'encombrement : aucun chevauchement, rien à moins de 0,3 mm du bord.
- Couronnes ≥ 0,40 mm partout ; C4 (empreinte LCSC Murata P5,0, couronne 0,35 mm) agrandi à 1,8 mm → 0,45 mm.
- 3D vérifiée : J3 / J4 / J5 entrée des fils vers le haut ; U1, C6, C7 abaissés de 1,3 mm (comme C1).

### 8.3 Routage

| Élément | Réalisé |
|---|---|
| Réseaux routés | 11, aucune liaison ouverte (tout refait, l'ancien routage de la carte chaudière a été effacé) |
| Largeurs | alimentation 0,6 mm, signaux 0,4 mm ; aucun via sur les signaux |
| Masse | plans `GND` sur les 2 faces (trait 0,254 mm), rayons thermiques, 39 vias de couture |
| Gouttes | posées, plans recoulés |
| Contrôle des règles | **0 erreur** ; schéma et PCB identiques |

Incident : juste après le routage automatique, EasyEDA s'est figé (66 %) au moment où les plans de masse étaient
recréés ; après redémarrage, les plans ont été recréés **une face à la fois** puis recoulés par Rebuild All sans problème.

### 8.4 Sérigraphie (face dessus, sans accents)

- Titre : « Capteur S3 » et « V 1.0 - Octobre 2026 ».
- Prises : « + PILES - » (J3), « INTER » (J4), « FLOTTEUR » et « FERME = PLEIN » (J5), « USB : INTER sur ARRET ».
- Reprises de la carte chaudière : « DHT20 » + broches (J1), « TCS34725 » + broches (J2), points de test, « XIAO
  ESP32-S3 + Wio », « CHAUDIERE » (vertical, contre la LED D1) ; « MCP1700 » sous U1.
- Contrôle : 52 textes (avec les repères), aucun sur une pastille, un via ou un trou, aucun chevauchement, une seule police.

### 8.5 Fiches personnelles utilisées

| Fiche (bibliothèque perso) | Repères | Remarque |
|---|---|---|
| **MCP1700-3302E TO-92 pas 2,54 (THT)** | U1 | **créée** (dossier Alimentation) : symbole LCSC C511277, empreinte commune avec le 2N2222A TO-92 pas 2,54 (pastilles 1,8 mm, trou 0,8 mm, pastille 1 carrée) |
| XIAO ESP32-S3 + Wio-SX1262 (THT) | A1 | reprise de la carte chaudière |
| Bornier XY301V 2P pas 5,0 agrandi (THT) | J3, J5 | reprise de NoeudChauffageSdb |
| JST-XH 2P / 4P / 5P (THT) | J4 / J1 / J2 | reprises |
| Résistance 1/4 W (THT) | R1–R5 | reprise |
| Condensateur électrolytique D5 pas 2,0 / D6,3 pas 2,5 (THT) | C2, C3 / C5 | reprises |
| LED 5 mm (THT), Point de test (THT) | D1, TP1–TP4 | reprises |

### 8.6 Reste à faire

- [x] Export `.epro2`, vues 2D / 3D / PCB et PDF dans `pcb/Capteur S3/`.
- [ ] Commit (procédure NAS de Pilotage_Maison, sans `global.h`).
- [ ] Commande JLCPCB.
- [ ] Code : calibre de la mesure des piles à 3,0 ; vérifier le sens de lecture du flotteur (1 = plein).
- [ ] Essais : redémarrages en Wi-Fi continu sur piles (C5), tension des piles neuves sur `VSW` (< 6,0 V ?).
- [ ] Boîtier imprimé : support 4 AA, interrupteur en paroi, passages de fils au bord haut.

## 9. Pièces LCSC (références EasyEDA)

| Pièce | Rôle | part_uuid |
|---|---|---|
| MCP1700-3302E/TO | U1 | 6a9589ea2d5f4668abdbf3f309da4501 |
| 1N5819 (YFW) | D2 | 1ee72050fd6a4fcc98af0c234119faba |
| RDER71H105K2K1H03B (Murata 1 µF) | C4 | 3d0f29dd7b434641a0d9199a9fa2dd57 |
| UVR1A471MED1TA (Nichicon 470 µF 10 V) | C5 | 5aa2be9fd5104c6e96b941a0c64720c2 |
| UVR1C100MDD (10 µF 16 V) | C2, C3 | 2f6c34bd2b064833b9ae0fcaed4dffac |
| C320C104K5R5TA (100 nF) | C1, C6, C7 | ae110dce71644cc0a180ce765b3811e3 |
| PCF14JT200K / 100K / 10K0 / 1K00 / 100R | R2 / R3 / R4 / R5 / R1 | b1f6db014fd841c193087e0bcc1e9c06 / cde41618505a435a9fbbc1f8cb9667c6 / f1eeb5691bc045a98708b6c70ada50f4 / dbfa2912556b4333a0a07dbcdaf2f715 / d10cc727db6343edb06ca77e0d1646f5 |
| XL-502UBC (LED bleue) | D1 | c3ab3a2f55bc4832809069ffaafbb617 |
| B4B-XH-A / B5B-XH-A | J1 / J2 | 7f13b11856f1454da74101a2bff96f83 / ab07958d1c8d468b9d479b7a885c6d02 |
