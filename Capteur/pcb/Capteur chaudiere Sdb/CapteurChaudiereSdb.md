# Capteur chaudière Sdb — Circuit imprimé

Version du 2026-10-19 (carte V 1.0 dessinée).
**État : schéma, placement, routage, gouttes et sérigraphie FAITS dans EasyEDA Pro ; contrôle des règles (DRC) = 0 erreur ;
schéma et PCB identiques. Prêt pour la commande** (détail au § 7).

Méthode et règles : celles de NoeudChauffageSdb (`pcb/NoeudChauffageSdb/NoeudChauffageSdb.md`) et des notices de
`D:\Developpement\EasyEda\Notices` (modèle de règles PCB, routage, bibliothèque personnelle).

## 1. Rôle du capteur et code

- Mesure la **température et l'humidité** de la salle de bain (DHT20).
- Surveille le **voyant de la chaudière** (TCS34725 collé contre le voyant) pour savoir si elle est allumée ou éteinte ;
  une LED bleue sur la carte recopie cet état.
- Remontée par **Wi-Fi / MQTT** ; le module LoRa Wio-SX1262 est monté mais inutilisé pour le moment (`_LORA_P2P_MODE_`
  désactivé).
- Alimentation par la **prise USB-C du XIAO** (bloc USB 5 V).

Code : `D:\Developpement\Pilotage_Maison\Capteur`, env PlatformIO `esp32s3` ; seule la configuration active de
`src/global.h` compte ici : `CAPTEUR_DHT20` + `CAPTEUR_RGB_TCS34725` + `LED_CAPTEUR_RGB`, `_WIFI_MODE_`.
Le nom NVS par défaut (`ThCh1er`) est remplacé par la configuration Web.

| Fonction | GPIO | Patte XIAO | Constante (`global.h`) | Remarque |
|---|---|---|---|---|
| I2C SDA | 5 | D4 | `DEFAULT_SDA_PIN` | bus partagé DHT20 + TCS34725 |
| I2C SCL | 6 | D5 | `DEFAULT_SCL_PIN` | idem |
| LED bleue (état chaudière) | 44 | D7 (RX) | `LED_CAPTEUR_RGB_PIN` | allumée à l'état haut (« DEL OFF » = 0 par défaut) |
| Interruption TCS34725 | 43 | D6 (TX) | `CAPTEUR_RGB_TCS34725_INTERRUPT` | facultative : **non câblée** (sortie série, messages au démarrage) |
| SPI du Wio-SX1262 | 7, 8, 9 | D8, D9, D10 | `LORA_SCK/MISO/MOSI_PIN` | occupées par le module LoRa : **laissées libres** sur la carte |
| DIO1, RESET, BUSY, CS du LoRa | 38 à 42 | connecteur du dessous du XIAO | `LORA_*_PIN` | ne passent pas par la carte |

Les broches I2C et la broche de la LED sont modifiables par la page Web (NVS) ; les valeurs ci-dessus sont celles par défaut.

Schéma de principe (vue de dessus du XIAO, USB-C en haut) :

```
                          USB-C (alimentation 5 V)
                       +-----------[====]-----------+
                       |    XIAO ESP32-S3 (A1)       |
                 n.c. -| D0                      5V |- n.c.
                 n.c. -| D1                     GND |------------------+---- GND
                 n.c. -| D2                     3V3 |----------+       |
                 n.c. -| D3                     D10 |- n.c.    |       |   (D8, D9, D10 :
      SDA  +-----------| D4 (GPIO5)              D9 |- n.c.    |       |    SPI du Wio-SX1262)
      SCL  | +---------| D5 (GPIO6)              D8 |- n.c.    |       |
           | |   n.c. -| D6 (GPIO43, INT)  D7 (GPIO44) |---+   |       |
           | |         +-----------------------------+   |   |       |
           | |          (Wio-SX1262 empilé au-dessus)     |   |       |
           | |                                            |   |       |
           | |   LED bleue :  D7 ---[R1 100 Ω]---|>|D1--- GND |       |
           | |                                            |   |       |
           | |   Découplage : 3V3 ---+--------+           3V3 |       |
           | |                      C1 100nF  C2 10µF (+)     |       |
           | |                       +--------+--- GND        |       |
           | |                                                |       |
           | |   J1 « vers DHT20 » (JST-XH 4)                 |       |
           | |      1 3V3 ----------------------------------- 3V3     |
           | |      2 GND ------------------------------------------- GND
           | +----- 3 SCL                                             |
           +------- 4 SDA                                             |
           | |                                                        |
           | |   J2 « vers TCS34725 » (JST-XH 5)                      |
           | |      1 3V3 ---> VIN du module ------------------ 3V3   |
           | |      2 GND ------------------------------------------- GND
           | +----- 3 SCL                                             |
           +------- 4 SDA                                             |
                    5 GND ---> LED du module (LED blanche éteinte) -- GND

     Points de test : TP1 = 3V3, TP2 = GND, TP3 = SDA, TP4 = SCL
     Modules : DHT20 (SDA, SCL, −, +) et TCS34725 (VIN, GND, 3V3 n.c., SCL, SDA, INT n.c., LED)
```

## 2. Décisions

| Sujet | Décision |
|---|---|
| Carte | XIAO ESP32-S3 surmonté du Wio-SX1262 (kit Seeed), sur barrettes femelles |
| Alimentation | prise USB-C du XIAO ; les modules prennent le 3,3 V du XIAO ; pas de régulateur ni de bornier sur la carte |
| DHT20 | module 4 broches (SDA, SCL, −, +), **sur fil de 10 cm**, fixé sur la paroi du boîtier (à l'air de la pièce) |
| TCS34725 | module type Adafruit 7 broches, **sur fil de 20 cm max.**, collé contre le voyant de la chaudière ; **alimenté par VIN** (régulateur et adaptateur de niveau I2C du module, comme le préconise Adafruit ; la broche 3V3 du module est une sortie, non câblée) ; **LED blanche éteinte** (broche LED reliée à GND sur la carte) |
| LED témoin | bleue 5 mm, au bord droit, **pattes pliées à 90°** pour pointer dans un trou de la paroi côté USB-C |
| Antennes | 2 antennes souples (Wi-Fi sur le XIAO, LoRa sur le Wio), dans le boîtier ou sorties |
| Boîtier | impression 3D, **fermé** (carte dans la salle de bain) |
| Composants | tout en traversant ; prises JST-XH |

## 3. Consommation estimée

| Élément | Courant |
|---|---|
| XIAO ESP32-S3 avec Wi-Fi | ≈ 100 mA en moyenne, pointes ≈ 350–500 mA (sous 5 V USB) |
| Wio-SX1262 | quelques µA en veille (LoRa inutilisé) ; jusqu'à ≈ 120 mA en émission si LoRa est activé un jour |
| DHT20 + TCS34725 (LED éteinte) | ≈ 1 mA sur le 3,3 V |
| LED bleue | ≈ 3 mA |

Un bloc USB 5 V de 1 A suffit largement. Le 3,3 V des modules (quelques mA) est fourni par le régulateur du XIAO.

## 4. Schéma

### 4.1 Page CARTE

| Repère | Composant | Pièce | Pattes → réseau |
|---|---|---|---|
| A1 | XIAO ESP32-S3 + Wio-SX1262 | fiche perso « XIAO ESP32-S3 + Wio-SX1262 (THT) » (même empreinte que le XIAO C3) | 3V3 → `3V3` ; GND → `GND` ; D4 → `SDA` ; D5 → `SCL` ; D7 → `LED_CHAUD` ; 5V, D0–D3, D6, D8–D10 non connectées |
| C1 | céramique 100 nF | C320C104K5R5TA | `3V3` → `GND` |
| C2 | électrolytique 10 µF 16 V | UVR1C100MDD (fiche perso « Condensateur électrolytique D5 pas 2,0 (THT) ») | + → `3V3` ; − → `GND` |
| R1 | 100 Ω | PCF14JT100R (fiche perso « Résistance 1/4 W (THT) ») | `LED_CHAUD` → `LED_CHAUD_R` |
| D1 | LED bleue 5 mm | XL-502UBC (fiche perso « LED 5 mm (THT) » : A = 1) | A → `LED_CHAUD_R` ; K → `GND` |

### 4.2 Page PÉRIPHÉRIQUES

| Repère | Composant | Pièce | Pattes → réseau |
|---|---|---|---|
| J1 | JST-XH 4 broches « vers DHT20 (sur le boîtier, 10 cm) » | B4B-XH-A(LF)(SN) (fiche perso « JST-XH 4P (THT) ») | 1 → `3V3` ; 2 → `GND` ; 3 → `SCL` ; 4 → `SDA` |
| J2 | JST-XH 5 broches « vers TCS34725 (voyant chaudière, 20 cm) » | B5B-XH-A(LF)(SN) (fiche perso « JST-XH 5P (THT) ») | 1 → `3V3` (VIN du module) ; 2 → `GND` ; 3 → `SCL` ; 4 → `SDA` ; 5 → `GND` (broche LED du module : LED blanche éteinte) ; ordre des broches du module, 3V3 et INT sautées |
| TP1–TP4 | points de test | fiche perso « Point de test (THT) » | `3V3`, `GND`, `SDA`, `SCL` |

Hors carte, non dessinés (connecteurs nommés « vers … ») : modules DHT20 et TCS34725, antennes.
Les deux modules ont leurs propres résistances de rappel I2C : aucune sur la carte. Les broches 3V3 (sortie de régulateur)
et INT du TCS34725 ne sont pas câblées.

## 5. Dimensionnement

- **R1** : (3,3 − ≈ 2,9) / 4 mA → 100 Ω (2 à 5 mA), comme la LED bleue de NoeudChauffageSdb.
- **Bus I2C** : 20 cm + 10 cm de fil ≈ 30 cm au total, plus les pistes : raisonnable à 100 kHz ; câbles torsadés
  conseillés (SDA avec GND, SCL avec 3V3).
- **Largeurs de piste** : pas de puissance sur la carte ; `3V3` et `GND` à 0,6 mm, signaux à 0,4 mm ; plan `GND` sur les
  2 faces.

## 6. Boîtier et mécanique

- **Impression 3D**, fermé : la carte est dans la salle de bain (humidité, vapeur).
- **Hauteur** : XIAO + Wio = 18 mm broches comprises ; sur barrettes femelles (≈ 8,5 mm), le haut du Wio arrive vers
  20–22 mm au-dessus de la carte → **hauteur intérieure ≥ 25 mm**.
- **USB-C** du XIAO au bord de la carte, en face d'une ouverture de la paroi.
- **DHT20** fixé à l'extérieur de la paroi par son trou (vis M3), fil de 10 cm passant par un trou rebouché.
- **TCS34725** : câble de 20 cm par un passe-fil ; le module se colle contre le voyant (adhésif double face, cache opaque
  conseillé pour éviter la lumière ambiante).
- **Antennes** : collées sur les parois intérieures, loin du plan de masse et l'une de l'autre (ou sorties).
- **LED bleue** : pattes pliées à 90°, elle pointe dans un **trou de la paroi côté USB-C** (bord droit de la carte).
- **Carte** : 46 × 41 mm, 4 trous M3 à 3,5 mm des coins (§ 7.2) ; l'USB-C, J1 et J2 sont sur **trois côtés différents**
  (droit, gauche, bas).

## 7. Circuit imprimé réalisé (EasyEDA Pro)

### 7.1 Projet

| Élément | Valeur |
|---|---|
| Projet | cloud (Online Mode) « Capteur chaudière Sdb » ; sauvegarde locale : `pcb/Capteur chaudiere Sdb/CapteurChaudiereSdb.epro2` |
| Schéma | « CapteurChaudiereSdb », 2 pages : Carte, Périphériques ; 11 composants, 6 réseaux |
| Règles | « THT soudure main JLCPCB » (modèle par défaut) |
| Classes | ALIM_3V3 0,6 mm (`3V3`, `GND`) ; signaux 0,4 mm |
| Exports | `pcb/Capteur chaudiere Sdb/` : 2D_PCB1_…png, 3D_PCB1_…png, PCB_PCB1_…png (vues de la carte), PDF_…zip |

### 7.2 Carte et placement

- **46 × 41 mm**, 2 couches ; **4 trous de fixation M3** non métallisés (Ø 3,2 mm) à 3,5 mm de chaque coin.
- **Bord droit** : prise USB-C du XIAO (alimentation) et LED bleue D1 (pliée vers le trou de la paroi).
- **Bord gauche** : J1 (DHT20) ; **bord bas** : J2 (TCS34725) ; entrée des fils tournée vers le bord
  (JST-XH : rotation 270° au bord gauche, 0° au bord bas).
- XIAO à droite, R1 au-dessus, C1 et C2 sous le XIAO près de sa broche 3V3, points de test en haut à gauche.
- Modèles 3D vérifiés (XIAO S3 tourné de −90° ; C1 abaissé de 1,3 mm par rapport au modèle LCSC pour toucher la carte).

### 7.3 Routage

| Élément | Réalisé |
|---|---|
| Réseaux routés | 5 (`3V3`, `SDA`, `SCL`, `LED_CHAUD`, `LED_CHAUD_R`), aucune liaison ouverte |
| Largeurs | `3V3` 0,6 mm, signaux 0,4 mm |
| Masse | plan `GND` coulé sur les 2 faces (trait de remplissage 10 mil), rayons thermiques, 27 vias de couture |
| Gouttes (teardrops) | posées (Tools → Teardrop), plans recoulés ensuite (Tools → Copper Manager → Rebuild All) |
| Contrôle des règles | **0 erreur** ; schéma et PCB identiques |

Piège rencontré : le script de routage créait les zones de masse avec un trait de **0,2 mil**, ce qui a figé EasyEDA au
recoulage ; zones recréées avec un trait de 10 mil.

### 7.4 Sérigraphie (face dessus)

- Titre : « Capteur Chaudiere Sdb » et « V 1.0 - Octobre 2026 » (sans accent : la police par défaut d'EasyEDA n'a pas
  les lettres accentuées et bascule sinon sur une autre police).
- Prises : « DHT20 » (J1 : 3V3, GND, SCL, SDA) ; « TCS34725 » (J2 : VIN, GND, SCL, SDA, LED, dans l'ordre du module).
- Points de test : 3V3, GND, SDA, SCL ; module : « XIAO ESP32-S3 + Wio » ; LED : « CHAUDIERE ».
- Contrôle : 30 textes, aucun sur une pastille, un via ou un trou, aucun chevauchement, une seule police.

### 7.5 Fiches personnelles utilisées

| Fiche (bibliothèque perso) | Repères | Remarque |
|---|---|---|
| XIAO ESP32-S3 + Wio-SX1262 (THT) | A1 | **créée** : symbole du XIAO C3 (mêmes noms de broches), empreinte « XIAO 14P THT (1-14) », 3D « ESP32-S3 » 21 × 17,8 mm (le Wio n'est pas représenté) |
| JST-XH 4P (THT) / JST-XH 5P (THT) | J1 / J2 | **créées** : pastilles 2,0 mm, trou 1,0 mm (pastille 1 ronde sur la 4P, comme l'empreinte LCSC) |
| Résistance 1/4 W (THT) | R1 | reprise de NoeudChauffageSdb |
| Condensateur électrolytique D5 pas 2,0 (THT) | C2 | idem |
| LED 5 mm (THT) | D1 | idem |
| Point de test (THT) | TP1–TP4 | idem |

### 7.6 Reste à faire

- [x] Export `.epro2` et vues dans `pcb/Capteur chaudiere Sdb/`.
- [ ] Commit (procédure NAS de Pilotage_Maison, sans `global.h`).
- [ ] Commande JLCPCB (5 cartes).
- [ ] Câbles : J2 vers **VIN** du TCS34725 (pas sa broche 3V3) ; torsader SDA/GND et SCL/3V3.
- [ ] Tracé du boîtier imprimé (hauteur intérieure ≥ 25 mm, trous USB-C et LED côté droit, passe-fils, DHT20 sur la paroi).

## 8. Pièces LCSC (références EasyEDA)

| Pièce | part_uuid |
|---|---|
| C320C104K5R5TA | ae110dce71644cc0a180ce765b3811e3 |
| UVR1C100MDD | 2f6c34bd2b064833b9ae0fcaed4dffac |
| PCF14JT100R | d10cc727db6343edb06ca77e0d1646f5 |
| XL-502UBC (bleue) | c3ab3a2f55bc4832809069ffaafbb617 |
| B4B-XH-A(LF)(SN) | 7f13b11856f1454da74101a2bff96f83 |
| B5B-XH-A(LF)(SN) | ab07958d1c8d468b9d479b7a885c6d02 |
