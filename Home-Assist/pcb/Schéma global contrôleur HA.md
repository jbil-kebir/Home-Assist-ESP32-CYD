# Contrôleur HA — Schéma global (version sans CYD)

Version du 2026-10-23. **État : schéma saisi dans EasyEDA (projet en ligne « Controleur HA »), circuit imprimé placé,
routé (DRC sans erreur) et sérigraphié (§ 7) ; pas encore fabriqué.**

Passage sur circuit imprimé du contrôleur primaire, testé jusqu'ici sur platine d'essai, puis boîtier imprimé.
Version **sans CYD** (la version avec écran reste sur platine d'essai). Méthode : celle de la borne Squash et des
capteurs (composants réels à pattes nommées, tableau validé avant pose, éléments hors carte non dessinés).

## 1. Rôle et code

| Carte | Rôle | Code |
|---|---|---|
| **ESP32-WROOM-32 DevKit** (38 broches, USB-C) | reçoit les données des capteurs (MQTT) et les remonte à Home Assistant ; reçoit les ordres de HA ou d'un CYD ; pilote lampes et chaudière en **RF 433 MHz** (CC1101) ; thermomètre local (DHT20) | `Home-Assist` (env `esp32dev`) |
| **XIAO ESP32-S3 + Wio-SX1262** (empilés) | passerelle **LoRa P2P ↔ MQTT** (chauffage de la salle de bain, capteurs LoRa) | `Relais-Lora` (env `seeed_xiao_esp32s3`) |

Les deux cartes communiquent **uniquement par Wi-Fi / MQTT** : sur le circuit imprimé, le XIAO ne partage que le
**5 V** et la masse. La radio LoRa est reliée au XIAO par le connecteur carte à carte du kit (rien à câbler).

**Réglages du code pour cette version** (à faire au montage, avec feu vert) dans `Home-Assist/src/global.h` :
`__LOCAL_MODE__` actif, `__CYD__` **commenté**, `__LOCAL_DS18B20__` **commenté** (thermomètre local = DHT20), et la
broche CSN du CC1101 distinguée selon la version (la version CYD garde GPIO 27, ses broches libres étant rares, et
GPIO 21 y est le rétroéclairage) :

```cpp
#ifdef __CYD__
#define CC1101_CS 27     // CYD : broche disponible sur son connecteur
#else
#define CC1101_CS 21     // sans CYD (carte EasyEDA) : rangée arrière du DevKit
#endif
```

### 1.1 Brochage de l'ESP32 (d'après `global.h`)

| Fonction | GPIO | Constante | Remarque |
|---|---|---|---|
| CC1101 SCK | 18 | `CC1101_SCK` | SPI (VSPI) |
| CC1101 MISO | 19 | `CC1101_MISO` | |
| CC1101 MOSI | 23 | `CC1101_MOSI` | |
| CC1101 CSN | **21** (27 sur la version CYD) | `CC1101_CS` | changé pour le routage (§ 7.2) |
| CC1101 GDO0 | 22 | `CC1101_GDO0` | émission et réception OOK ; GDO2 non utilisé |
| DHT20 SDA | 5 | `DEFAULT_DHT20_SDA_PIN` | broche de démarrage : doit être haute au démarrage (tirage du module DHT20 : correct) |
| DHT20 SCL | 26 | `DEFAULT_DHT20_SCL_PIN` | |

Les broches 4, 16, 17 (LED RGB) et 21, 25, 32, 33, 36, 39 (écran) ne servent qu'avec le CYD : non câblées.

## 2. Schéma global

```
HORS CARTE     │                    SUR LE CIRCUIT IMPRIMÉ                  │ HORS CARTE
               │                                                            │
bloc 12 V ═════╪═► J1 jack ─►|─ DP 1N5819 ─┬─ J2 ═══════════════════════════╪══► interrupteur
(5,5 × 2,1)    │                           └─ J2 ◄══════════════════════════╪═══ MTS-102
               │                              │ 12V_SW                      │
               │                         C1 C2┴─► A1 MP1584EN (→ 5,0 V)      │
               │                                    │ +5V  C3 C4            │
               │              ┌─────────────────────┴──────────┐            │
               │      DA 1N5819 ►|                     DB 1N5819 ►|         │
               │              │ 5V_ESP  C5                     │ 5V_XIAO  C6 C7
               │      A2 ESP32 DevKit (5V)            A3 XIAO S3 + Wio-SX1262
               │              │ 3V3                    (antennes Wi-Fi, LoRa)
               │              ├─► C8 C9 ─► A4 CC1101 VCC ── SMA ════════════╪══► antenne 433
               │   GPIO 18 19 23 21 22 ──► A4 SCK MISO MOSI CSN GDO0         │
               │              └─► J3 3V3 ═══════════════════════════════════╪══► DHT20
               │   GPIO 26 (SCL), 5 (SDA) ──► J3                             │   (paroi)
               │
               │   +5V ──[R1 1 kΩ]── LED_A ─► J4 ════════════════════════════╪══► LED « allumé »
               │                                                            │   (face avant)
               │   Masse commune : J1, A1, A2, A3, A4, J3, J4.
```

## 3. Détail par fonction

### 3.1 Entrée 12 V et interrupteur

- **J1** : jack femelle 5,5 × 2,1 mm **soudé sur la carte**, au bord, face à un trou du boîtier (jamais débranché).
  Centre = + (vérifié sur le bloc).
- **DP** (1N5819) en série : protège contre un bloc à polarité inversée (chute ≈ 0,3 V, sans conséquence à 12 V).
- **J2** (JST-XH 2) : interrupteur MTS-102 en façade, **avant** le convertisseur : il coupe tout. J2 est à l'**arrière**
  de la carte, près du jack et du MP1584EN (le 12 V ne traverse pas la carte) ; les fils (≈ 15 cm) vont à l'interrupteur.

### 3.2 Conversion 12 V → 5 V

- **A1** : module abaisseur **MP1584EN** (réglable, 3 A), **réglé à 5,0 V au voltmètre avant de brancher les
  cartes**, monté sur barrettes femelles 1 broche (IN+, IN−, OUT+, OUT−), comme le MT3608 de la borne.
- Pas de régulateur linéaire 7805 : de 12 V à 5 V, il dissiperait environ 3,5 W dans le boîtier.
- **C1** (100 µF 25 V) + **C2** (100 nF) à l'entrée, **C3** (220 µF 16 V) + **C4** (100 nF) à la sortie.

**Budget de courant** (estimation) : ESP32 en Wi-Fi ≈ 250 mA en pointe, XIAO S3 en Wi-Fi + SX1262 en émission
≈ 350 mA, CC1101 en émission ≈ 35 mA : **moins de 0,7 A sur le 5 V**, soit ≈ 0,35 A sur le 12 V (bloc 12 V 1 A :
large marge).

### 3.3 Distribution du 5 V

- **DA** (1N5819) vers la broche 5V du DevKit, **DB** (1N5819) vers la broche 5V du XIAO, comme la diode DA de la
  borne : brancher un câble USB pour programmer l'une ou l'autre carte, montage alimenté ou non, ne renvoie pas de
  courant vers le convertisseur ni vers l'autre carte. Chaque carte reçoit ≈ 4,7 V : suffisant pour leurs
  régulateurs 3,3 V.
- **C5** (100 nF) au pied du DevKit, **C6** (10 µF 16 V) + **C7** (100 nF) au pied du XIAO.

### 3.4 CC1101

- **A4** : module CC1101 **EBYTE E07-M1101D** (« V2.0 433M », 28 × 15 mm, 2 × 4 broches au pas de 2,54 mm, prise SMA dans l'axe, antenne à visser), sur
  **barrette femelle 2 × 4** (démontable), alimenté par le **3V3 du DevKit** comme sur la platine d'essai.
- **C8** (100 µF 25 V, celui de la platine d'essai) + **C9** (100 nF) au plus près de sa broche VCC : réserve pour
  les appels de courant pendant les émissions.
- Brochage (photos du vendeur, `U:\Info-Developpement\Composants\RF\CC1101 (E07-M1101D)\`) : 1 GND, 2 VCC,
  3 GDO0, 4 CSN, 5 SCK, 6 MOSI, 7 MISO, 8 GDO2 (non relié).
- La prise SMA au bord de la carte, l'antenne sort par la paroi.

### 3.5 DHT20 déporté

- **J3** (JST-XH 4) « vers DHT20 », **même brochage que les capteurs** : 1 `3V3`, 2 `GND`, 3 `SCL`, 4 `SDA`.
  Le module DHT20 (avec ses résistances de tirage) est fixé sur la paroi extérieure du boîtier, loin de la chaleur
  des cartes.

### 3.6 Voyant « allumé »

- LED 5 mm verte sur la **face avant**, à côté de l'interrupteur, reliée par **J4** (JST-XH 2).
- **R1** (1 kΩ) sur la carte, alimentée par le **+5V** (après l'interrupteur et le convertisseur) : la LED ne s'allume que
  si le 5 V est réellement présent, ce qui contrôle aussi le MP1584EN. Courant ≈ 3 mA.

### 3.7 Antennes

- **ESP32** : antenne gravée sur le module WROOM → le bout du module **dépasse du bord de la carte** (ou zone sans
  cuivre sous l'antenne).
- **XIAO S3** : antenne Wi-Fi souple 40 × 20 mm (câble u.FL de 52 mm), collée contre la paroi gauche, à l'intérieur.
- **Wio-SX1262** : antenne LoRa souple du kit, 40 × 10 mm (câble u.FL de 52 mm), collée en haut de la paroi droite, à
  l'intérieur (décidé le 2026-10-24 : plus de SMA de traversée ni d'antenne 868 MHz externe).
- **CC1101** : antenne SMA (§ 3.4).

## 4. Connecteurs

| Repère | Type | Broches | Vers |
|---|---|---|---|
| J1 | Jack femelle 5,5 × 2,1 mm pour circuit imprimé | centre +12 V, manchon GND | bloc secteur 12 V |
| J2 | JST-XH 2 | 1 `12V`, 2 `12V_SW` | interrupteur MTS-102 |
| J3 | JST-XH 4 | 1 `3V3`, 2 `GND`, 3 `SCL`, 4 `SDA` | DHT20 |
| J4 | JST-XH 2 | 1 `LED_A` (anode), 2 `GND` (cathode) | LED « allumé » |

## 5. Nomenclature

Liste d'achat détaillée (références, pièces non dessinées, hors carte) : `Controleur_Composants.md` ; export EasyEDA
des composants dessinés : `Controleur_BOM.xlsx`.

| Repère | Composant | Qté |
|---|---|---|
| A1 | Module abaisseur MP1584EN, réglé à 5,0 V, sur 4 barrettes femelles 1 broche | 1 |
| A2 | ESP32-WROOM-32 DevKit 38 broches USB-C, sur 2 barrettes femelles 1 × 19 | 1 |
| A3 | XIAO ESP32-S3 + Wio-SX1262 (kit), sur 2 barrettes femelles 1 × 7 | 1 |
| A4 | Module CC1101 EBYTE E07-M1101D + antenne SMA, sur barrette femelle 2 × 4 | 1 |
| DP, DA, DB | Diode Schottky 1N5819 | 3 |
| C1, C8 | Électrolytique 100 µF 25 V | 2 |
| C3 | Électrolytique 220 µF 16 V | 1 |
| C6 | Électrolytique 10 µF 16 V | 1 |
| C2, C4, C5, C7, C9 | Céramique 100 nF, pas de 2,54 mm | 5 |
| R1 | Résistance 1 kΩ 1/4 W | 1 |
| J1 | Jack femelle 5,5 × 2,1 mm pour circuit imprimé | 1 |
| J2 | Embase JST-XH 2 broches | 1 |
| J3 | Embase JST-XH 4 broches | 1 |
| J4 | Embase JST-XH 2 broches | 1 |
| – | Hors carte : bloc 12 V 1 A, interrupteur MTS-102, LED 5 mm verte, module DHT20, antenne 433 MHz SMA, antennes souples Wi-Fi et LoRa (u.FL, fournies) | – |

Fiches EasyEDA réutilisables : XIAO ESP32-S3 + Wio-SX1262 (carte Capteur S3), 1N5819, condensateurs, JST-XH
(borne Squash). À créer ou à chercher : DevKit ESP32 38 broches, module CC1101 2 × 4, module MP1584EN, jack.

## 6. Vérifications avant la saisie

1. ~~Cotes du DevKit 38 broches~~ : mesurées (55 × 28 mm, rangées à 25,5 mm, antenne 4,5 mm au-delà de la dernière
   broche, prise USB-C 6,5 mm) — `U:\Info-Developpement\Composants\Cartes\ESP32 DevKit (WROOM)\Mesures.md`.
2. ~~Brochage du CC1101~~ : confirmé (photos du vendeur) ; position du connecteur mesurée
   (`U:\Info-Developpement\Composants\RF\CC1101 (E07-M1101D)\Mesures.md`).
3. ~~Cotes du MP1584EN~~ : d'après le modèle GrabCAD (8 trous, 2 par borne ; vue de dessus IN à droite, OUT à gauche,
   + en haut) — **à vérifier sur le module reçu** (`U:\Info-Developpement\Composants\Alimentation\MP1584EN\Mesures.md`).
4. ~~Polarité du bloc 12 V~~ : centre +.
5. **Forme de la carte et du boîtier** (décidé le 2026-10-23) : boîtier posé sur une étagère.
   - **Avant** : interrupteur et LED (fils ; J4 au bord avant de la carte, J2 à l'arrière, voir § 3.1).
   - **Arrière** : jack 12 V (J1 au bord), SMA du CC1101 (module au bord, la prise traverse la paroi).
     Boîtier dessiné : `Home-Assist/boitier/Boîtier contrôleur HA.md`.
   - **Côté droit** : fentes USB du DevKit et du XIAO (programmation sans ouvrir le boîtier).
   - **Côté gauche** : antenne gravée du DevKit (dépasse du bord, sans cuivre dessous), fentes d'aération, DHT20 contre
     la paroi, loin du MP1584EN.

## 7. Circuit imprimé (état au 2026-10-23 : placé, routé, sérigraphie complète)

Projet EasyEDA Pro **en ligne** « Controleur HA » (pages Alimentation, Carte, Voyant), copie exportée
`Controleur_2026-10-23.epro2` avec les vues `Controleur_2D/3D/PCB_2026-10-23.png` et les schémas
`Controleur_Schemas_2026-10-23.zip`. Fabrication prévue comme la borne Squash (JLCPCB, 2 couches, 1,6 mm, HASL,
tout traversant soudé à la main).

### 7.1 Carte et placement

Carte de **57 × 84 mm**, origine au coin avant gauche. 4 trous M3 (Ø 3,2, non métallisés) : coins avant gauche,
avant droit et arrière droit, et **bord gauche** à 47 mm de l'avant (le coin arrière gauche est occupé par le CC1101).

```
                         ARRIÈRE : jack 12 V, antennes
   ┌────────────────────────────────────────────────────────┐
   │ ┌─────────────┐  J1 jack    ┌────────────────┐    (M3) │
   │ │ A4 CC1101   │  J1 12V     │ A1 MP1584EN    │         │
   │ │ (module     │  DP         │ OUT       IN   │         │
   │ │  15 × 28)   │             └────────────────┘         │
   │ │             │ DB  C1 C2        INTER J2 [J2]         │
   │ └──broches────┘                ┌──────────────────┐     │
   │(M3) C9 C8      DA  C3 C4       │ A3 XIAO S3 + Wio │ USB→│
   │                    C6 C7       └──────────────────┘     │
   │ ┌────────────────────────────────────────────────────┐ │
 ANT│ │ A2 ESP32 DevKit 38 broches                         │ USB→
   │ └────────────────────────────────────────────────────┘ │
   │(M3) J3 DHT20      J4 LED      C5  R1              (M3)  │
   └────────────────────────────────────────────────────────┘
                     AVANT : interrupteur, LED (sur fils)
```

| Élément | Position |
|---|---|
| A2 DevKit | sur toute la largeur ; prise USB-C au bord droit (dépasse d'environ 1,7 mm), antenne au bord gauche |
| A3 XIAO | à droite, prise USB-C alignée sur celle du DevKit (dépasse de 1,5 mm) |
| A4 CC1101 | arrière gauche ; le corps du module (15 × 28 mm, contour en sérigraphie) au ras du bord arrière, la prise SMA traverse la paroi |
| A1 MP1584EN | arrière, entrée IN à droite, près de J2 |
| J1 jack | bord arrière, ouverture vers l'arrière (vérifié en 3D) |
| J2 interrupteur | arrière droite, entre le XIAO et le MP1584EN |
| J3 DHT20, J4 LED | bord avant |

Modèles 3D : XIAO « XIAO-ESP32S3 v3 », DevKit « comm-th_esp32-devkitc-32ue » (système, agrandi à 55 × 28 mm),
MP1584EN = modèle GrabCAD ; pas de modèle pour le CC1101 (contour seul).

### 7.2 Routage

Routage automatique (Copilot) puis plans de masse recréés à la main, face par face : **DRC sans erreur**.

| Règle | Valeur |
|---|---|
| Puissance (12V_IN, 12V, 12V_SW, +5V, 5V_ESP, 5V_XIAO) | pistes 1,0 mm (min. 0,8) |
| 3V3 | pistes 0,6 mm (min. 0,5) |
| Signaux (CC_SCK, CC_MISO, CC_MOSI, CC_CSN, CC_GDO0, SDA, SCL, LED_A) | pistes 0,4 mm (min. 0,3) |
| GND | plans sur les deux faces, connexions à rayons thermiques, 59 vias de couture |
| Écarts | 0,3 mm ; 0,5 mm au bord ; vias 0,8 mm, perçage 0,4 mm |
| Antenne du DevKit | zone **sans cuivre** de 9 × 22 mm au bord gauche, entre les deux rangées de broches |

Un seul via de signal (CC_CSN). Le DevKit occupe toute la largeur et ses broches (pas de 2,54 mm) ne laissent pas
passer de piste entre elles : rien ne doit traverser sa bande. D'où deux choix faits au routage : **J2 à l'arrière**
(12 V et 12V_SW restent côté alimentation) et **CSN sur GPIO 21** (rangée arrière, côté CC1101) au lieu de GPIO 27.

### 7.3 Sérigraphie

Repères hors des contours, 1,1 mm, droits (DA à 90°, diode verticale). Noms des connecteurs (1,1 mm) et des broches
(1,0 mm) : « J1 12V », « INTER J2 », « J3 DHT20 » + 3V3 GND SCL SDA, « J4 LED » + / −. Contours des modules A1, A2 et A4.

**Suite** : fichiers de fabrication (Gerber) et commande ; boîtier imprimé ; `global.h` au flashage (§ 1).
