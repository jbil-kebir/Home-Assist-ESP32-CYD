# NoeudChauffageSdb — Circuit imprimé

Version du 2026-10-18 (carte V 1.0 dessinée) ; mise à jour le 2026-10-24 (LED IR IR333C-A, boîtier tracé : § 6).
**État : schéma, placement, routage, gouttes et sérigraphie FAITS dans EasyEDA Pro ; contrôle des règles (DRC) = 0 erreur ;
schéma et PCB identiques. Prêt pour l'export .epro2 et la commande** (détail au § 8).

Notices à appliquer (dossier `D:\Developpement\EasyEda\Notices`) : modèle de règles PCB, routage PCB, bibliothèque
personnelle. La carte est conçue **de zéro**, dimensionnée pour ses besoins réels (les blocs d'alimentation
réutilisables seront vus plus tard) ; les mesures faites à l'usage (§ 7) diront les vraies limites de puissance.

## 1. Rôle du nœud et code

- Commande le chauffage de la salle de bain par **infrarouge** (protocole NEC) depuis la domotique (MQTT).
- Vérifie que l'ordre a été reçu en écoutant les **bips** du chauffage (≈ 3 950 Hz) avec un micro MAX4466.
- Écoute continue (pas de veille profonde) → **alimentation secteur** par bloc 12 V.

Code : `D:\Developpement\Pilotage_Maison\Capteur`, projet multi-modules ; seule la configuration active de
`src/global.h` compte ici : `CAPTEUR_MICRO_MAX4466` + `ACTIONNEUR_IR`, carte XIAO ESP32-C3 (env `esp32c3`).

| Fonction | GPIO | Patte XIAO | Constante (`global.h`) | Remarque |
|---|---|---|---|---|
| Micro MAX4466 | 3 | D1 | `CAPTEUR_MICRO_MAX4466_PIN` | entrée analogique 12 bits, atténuation 11 dB (0–3,3 V) |
| LED bleue (son) | 7 | D5 | `CAPTEUR_MICRO_MAX4466_LED_PIN` | flash à chaque bip valide ; allumée à l'état haut |
| Émetteur IR | 20 | D7 | `ACTIONNEUR_IR_PIN` | pas de rôle au démarrage (GPIO8 et GPIO21 évitées volontairement) |
| LED rouge (IR) | 10 | D10 | `ACTIONNEUR_IR_LED_PIN` | allumée pendant l'émission ; état haut |

Les broches du micro et de l'IR sont modifiables par la page Web (NVS) ; les valeurs ci-dessus sont celles par défaut.

## 2. Décisions

| Sujet | Décision |
|---|---|
| Carte | XIAO ESP32-C3 sur barrettes femelles (fiche personnelle « XIAO ESP32-C3 (THT) ») |
| Alimentation | bloc secteur 12 V (tension à vide < 16 V) → bornier → régulateur à découpage 5 V |
| Interrupteur | aucun (nœud fixe) |
| Micro et LED IR | sur FIL (placés face au chauffage et à son haut-parleur), prises JST-XH |
| LED IR | 2 × **Everlight IR333C-A** (5 mm, 940 nm) en série ; les HIR204C de l'IR Box sont des 3 mm |
| Transistor IR | 2N2222A |
| LED témoins | sur la carte, visibles par 2 trous du couvercle (tubes-guides) ; LED bleue gardée |
| Boîtier | **impression 3D**, tracé le 2026-10-24 (§ 6) ; micro et LED IR à l'intérieur, sous le couvercle ; Hammond 1554C2GYCL abandonné |
| Composants | tout en traversant |

## 3. Consommation estimée (à mesurer, § 7)

| Élément | Courant sous 5 V |
|---|---|
| XIAO avec Wi-Fi | ≈ 100 mA en moyenne, pointes ≈ 350 mA |
| LED IR | ≈ 100 mA en impulsions (§ 5.2) |
| LED témoins + micro | < 10 mA |
| **Total en pointe** | **≈ 0,5 A** (≈ 0,25 A côté 12 V) |

Le régulateur choisi (1 A) laisse environ deux fois la pointe estimée.

## 4. Schéma

### 4.1 Page ALIMENTATION

| Repère | Composant | Pièce | Pattes → réseau |
|---|---|---|---|
| J1 | bornier 2 broches pas 5 mm « vers bloc secteur 12 V » | XY301V-A-5.0-2P | 1 → `V12_J` ; 2 → `GND` |
| D1 | Schottky 1 A, protection contre l'inversion | 1N5819 | A → `V12_J` ; K → `V12` |
| C1 | électrolytique 47 µF 25 V | 25YXF47MEFC5X11 | + → `V12` ; − → `GND` |
| U1 | régulateur à découpage 5 V 1 A (8–28 V), brochage du 7805, ne chauffe pas | Recom R-78E5.0-1.0 | 1 +Vin → `V12` ; 2 → `GND` ; 3 +Vout → `5V_REG` |
| JP1 | barrette mâle 2 broches + cavalier, mesure du courant total | barrette 1 × 2 | 1 → `5V_REG` ; 2 → `+5V` |
| C6 | électrolytique 100 µF 16 V | KM101M016E110A | + → `+5V` ; − → `GND` |
| C7 | céramique 100 nF | C320C104K5R5TA | `+5V` → `GND` |

### 4.2 Page CARTE

| Repère | Composant | Pièce | Pattes → réseau |
|---|---|---|---|
| A1 | XIAO ESP32-C3 | fiche perso « XIAO ESP32-C3 (THT) » | 5V → `5V_CARTE` ; GND → `GND` ; 3V3 → `3V3` ; D1 → `MICRO` ; D5 → `LED_SON` ; D7 → `IR_CMD` ; D10 → `LED_IR` ; autres non connectées |
| DA | Schottky 1 A : empêche l'USB du XIAO de remonter dans le +5V | 1N5819 | A → `+5V` ; K → `5V_CARTE` |
| C8 | électrolytique 10 µF 16 V | UVR1C100MDD | + → `5V_CARTE` ; − → `GND` |

### 4.3 Page PÉRIPHÉRIQUES

Micro :

| Repère | Composant | Pièce | Pattes → réseau |
|---|---|---|---|
| J5 | JST-XH 3 broches « vers micro MAX4466 » | B3B-XH-A(LF)(SN) | 1 → `3V3` ; 2 → `GND` ; 3 → `MICRO` |
| C9 | céramique 100 nF, découplage du micro | C320C104K5R5TA | `3V3` → `GND` |

Le micro est alimenté en 3,3 V : sa sortie est centrée sur VCC / 2 et l'entrée analogique ne monte qu'à 3,3 V.

Émission IR :

| Repère | Composant | Pièce | Pattes → réseau |
|---|---|---|---|
| Q1 | NPN | 2N2222A (TO-92, E-B-C) | E → `GND` ; B → `IR_B` ; C → `IR_K` |
| R1 | 220 Ω, base | PCF14JT220R | `IR_CMD` → `IR_B` |
| R2 | 10 kΩ, LED IR éteinte au démarrage | PCF14JT10K0 | `IR_B` → `GND` |
| JP2 | barrette mâle 2 broches + cavalier, mesure du courant IR | barrette 1 × 2 | 1 → `+5V` ; 2 → `IR_V` |
| R3 | 22 Ω 1/4 W, série LED (montée un peu au-dessus de la carte pour essais) | PCF14JT22R0 | `IR_V` → `IR_A` |
| J6 | JST-XH 2 broches « vers LED IR (2 × HIR204C) » (LED posées : 2 × IR333C-A) | B2B-XH-A(LF)(SN) | 1 → `IR_A` ; 2 → `IR_K` |
| C10 | électrolytique 100 µF 16 V, réserve pour les impulsions IR | KM101M016E110A | + → `+5V` ; − → `GND` |

LED témoins :

| Repère | Composant | Pièce | Pattes → réseau |
|---|---|---|---|
| R4 | 100 Ω | PCF14JT100R | `LED_SON` → `LED_SON_R` |
| D6 | LED bleue 5 mm | XL-502UBC (attention : K = 1, A = 2) | A → `LED_SON_R` ; K → `GND` |
| R5 | 330 Ω | PCF14JT330R | `LED_IR` → `LED_IR_R` |
| D7 | LED rouge 5 mm | XL-502SURC (A = 1, K = 2) | A → `LED_IR_R` ; K → `GND` |

Points de test (pastilles) : `V12`, `+5V`, `3V3`, `MICRO`, `IR_K` (collecteur : impulsions IR), `GND`.

Hors carte, non dessinés (connecteurs nommés « vers … ») : bloc secteur 12 V, module micro MAX4466,
2 LED IR en série (IR333C-A ; le PCB garde le nom HIR204C dans le libellé de J6).

## 5. Dimensionnement

### 5.1 Alimentation

- D1 : 0,25 A côté 12 V → 1N5819 (1 A) suffit.
- U1 : R-78E5.0-1.0 (1 A) ≈ 2 × la pointe estimée ; un régulateur linéaire 7805 dissiperait (12 − 5) × 0,5 = 3,5 W :
  exclu.
- DA : même rôle que sur la borne Squash (XIAO alimenté par sa broche 5V, USB branché pour le reflashage).

### 5.2 Émission IR

LED posées : **IR333C-A** (fiche technique : `U:\Info-Developpement\Composants\LED\IR333C-A`) : 940 nm, angle 20°,
100 mA en continu, VF ≈ 1,4 V à 100 mA. Le 1 A de la fiche technique ne vaut que pour des impulsions ≤ 100 µs et ≤ 1 %
du temps, ce que n'est pas le NEC (porteuse 38 kHz allumée environ un tiers du temps pendant les rafales). Le montage
d'essai actuel (HIR204C) fonctionne à 1,80 m avec environ 15 mA (6 V, 197 Ω en série, 150 Ω en base, 2N3511). Cible :
**≈ 90 à 100 mA en crête** (≈ 6 fois plus de marge de portée, dans la limite du continu des LED).

| Repère | Calcul | Valeur | IR Box ESP32 1.0 |
|---|---|---|---|
| R3 (série LED) | (5 − 2 × 1,4 − 0,2) / 0,1 A = 20 Ω | 22 Ω 1/4 W (≈ 90 mA) | 5 Ω (≈ 0,35 A) |
| R1 (base) | courant de base = 100 mA / 10 = 10 mA ; (3,3 − 0,8) / 10 mA = 250 Ω | 220 Ω (≈ 11 mA) | 197 Ω |
| R2 (rappel) | LED IR éteinte tant que la broche n'est pas pilotée | 10 kΩ | absente |
| R4 (LED bleue) | (3,3 − ≈ 2,9) / 4 mA | 100 Ω (2 à 5 mA) | — |
| R5 (LED rouge) | (3,3 − 1,9) / 4 mA | 330 Ω (≈ 4 mA) | — |

Sécurité : si la broche IR restait bloquée à l'état haut, les LED recevraient ≈ 90 mA en continu (sous leur limite de
100 mA) et R3 dissiperait 0,18 W (< 0,25 W). Avec les 5 Ω de l'IR Box, ce serait 0,35 A en continu dans les LED.

Pour aller plus loin si la portée manque : 15 Ω → ≈ 135 mA en crête (encore raisonnable, en 1/2 W). Le faisceau
des IR333C-A est étroit (20°) : le couvercle du boîtier doit regarder le chauffage.

Défaut de l'IR Box à NE PAS reprendre : LED L1 + 100 Ω entre la base et la masse (vole le courant de base, ne s'allume
presque pas). Ici la LED témoin a sa propre broche.

## 6. Boîtier et mécanique

**Boîtier imprimé en 3D, tracé le 2026-10-24** : `Capteur/boitier/NoeudChauffageSdb/` (source `boitier_noeud.scad`,
STL, aperçus, fiche « Boîtier NoeudChauffageSdb.md » avec le détail, le montage et les cotes à vérifier avant impression).

- Fixé à plat au mur de la salle de bain (à l'abri), par 4 oreilles ; **couvercle face au chauffage** (≈ 1,80 m).
- Carte tournée de 180° : prise USB-C à gauche (fente), J1 / J5 / J6 en haut, LED témoins en bas ; placement inchangé.
- Seul le câble 12 V sort, par la paroi droite (fente + serre-câble).
- **Micro et LED IR à l'intérieur**, sous le couvercle : micro vissé (M2) derrière une grille de 7 trous, LED IR passées
  dans 2 trous du couvercle (le PETG opaque arrête l'infrarouge).
- LED témoins D6 / D7 visibles par 2 trous du couvercle : **les souder avec 16,4 mm de patte** entre la carte et la
  collerette (tubes-guides du couvercle).
- Antenne Wi-Fi souple 40 × 20 mm contre la paroi basse.

Le boîtier Hammond 1554C2GYCL envisagé au départ est abandonné.

## 7. Mesures à l'usage (limites de puissance)

But : connaître les vraies limites hautes en puissance pour dimensionner les cartes suivantes.

| Mesure | Où | Comment |
|---|---|---|
| Courant total 5 V | JP1 | retirer le cavalier, ampèremètre à sa place ; noter repos, Wi-Fi, émission IR |
| Courant IR | JP2 | idem pendant une émission (valeur moyenne à l'ampèremètre, crête à l'oscilloscope sur R3) |
| Impulsions IR | point de test `IR_K` | oscilloscope : forme et tension de saturation du 2N2222A |
| Tensions | points de test `V12`, `+5V`, `3V3` | stabilité pendant les pointes Wi-Fi et IR |
| Portée IR | — | essais de R3 : 47 Ω, puis 22 Ω, puis 15 Ω, à la distance réelle (1,80 m) |
| Échauffement | U1, R3, Q1 | au doigt ou thermomètre après une série d'émissions |

## 8. Circuit imprimé réalisé (EasyEDA Pro)

### 8.1 Projet

| Élément | Valeur |
|---|---|
| Projet | cloud (Online Mode) « NoeudChauffageSdb » ; sauvegarde locale : `pcb/NoeudChauffageSdb/NoeudChauffageSdb.epro2` |
| Schéma | « NoeudChauffageSdb », 3 pages : Alimentation, Carte, Périphériques ; 29 composants, 16 réseaux, 60 liaisons ; pages aérées (composants regroupés par fonction, dans le sens du signal) |
| Exports | `pcb/NoeudChauffageSdb/` : 2D_PCB_…png, 3D_PCB_…png, PCB_PCB_…png (vues de la carte), PDF_…zip (schéma et PCB en PDF) |
| Règles | « THT soudure main JLCPCB » (notice du modèle de règles) |
| Classes | PUISSANCE 1,0 mm (`V12_J`, `V12`, `5V_REG`, `+5V`, `5V_CARTE`, `GND`, `IR_V`, `IR_A`, `IR_K`) ; ALIM_3V3 0,6 mm (`3V3`) ; signaux 0,4 mm |
| Couronnes | ≥ 0,4 mm partout (fiches personnelles du § 8.5), pastilles THT 1,8 mm sur les composants courants |

### 8.2 Carte et placement

- **64 × 48 mm**, 2 couches ; **4 trous de fixation M3** non métallisés (Ø 3,2 mm) à 3,5 mm de chaque coin.
- **Bord bas** : tout ce qui part sur fil ou se mesure à la pince : J1 (12 V), J6 (LED IR), J5 (micro), points de
  test `MICRO` et `GND`. Le bornier J1 et la prise J5 sont tournés de 180° pour que **l'entrée des fils soit
  tournée vers le bord** : sur J1, le + est à droite et le − à gauche ; sur J5, de gauche à droite : MICRO, GND, 3V3.
- **Bord droit** : prise USB-C du XIAO, accessible pour le reflashage.
- **Bord haut** : LED témoins D6 (bleue, son) et D7 (rouge, IR).
- Alimentation à gauche (J1 → D1 → C1 → U1 → JP1), émission IR au centre (JP2, R3, C10, Q1, R1, R2), XIAO à droite.
- Modèles 3D vérifiés (XIAO tourné de −90°, composants posés à plat sur la carte).

### 8.3 Routage

| Élément | Réalisé |
|---|---|
| Réseaux routés | 16, aucune liaison ouverte |
| Largeurs | puissance 1,0 mm, `3V3` 0,6 mm, signaux 0,4 mm |
| Masse | plan `GND` coulé sur les 2 faces, rayons thermiques sur les pastilles, 29 vias de couture |
| Gouttes (teardrops) | posées sur toutes les pastilles et vias (Tools → Teardrop), plans recoulés ensuite (Tools → Copper Manager → Rebuild All) |
| Contrôle des règles | **0 erreur** ; schéma et PCB identiques (60 liaisons) |

### 8.4 Sérigraphie (face dessus)

- Titre : « Noeud Chauffage Sdb » et « V 1.0 - Octobre 2026 ».
- Prises : « 12 V » (J1, broches + et −), « LED IR » (J6, broches + et −), « MICRO » (J5, broches MICRO, GND, 3V3 de gauche à droite).
- Cavaliers de mesure : « I 5V » (JP1, courant total 5 V) et « I IR » (JP2, courant des LED IR) ; « I » = intensité.
- Points de test : V12, +5V, 3V3, MICRO, IR_K, GND (à côté des repères TP1 à TP6).
- Modules : « XIAO ESP32-C3 » (A1) et « R-78E5.0-1.0 » (U1).
- Contrôle : aucun texte sur une pastille, un via ou un trou ; hauteur mini 0,81 mm (noms des broches de J5).

### 8.5 Fiches personnelles utilisées

| Fiche (bibliothèque perso) | Repères | Pourquoi |
|---|---|---|
| XIAO ESP32-C3 (THT) | A1 | barrettes femelles, pattes numérotées 1–14, pastilles 1,8 mm |
| Bornier XY301V 2P pas 5,0 agrandi (THT) | J1 | couronne agrandie (pastille 2,4 / trou 1,2) |
| 2N2222A TO-92 pas 2,54 (THT) | Q1 | pattes écartées pour la soudure à la main (1 = E, pastille carrée) |
| Résistance 1/4 W (THT) | R1–R5 | une seule empreinte pas 10,16 mm, pastille 1,8 mm |
| Condensateur électrolytique D5 pas 2,0 (THT) | C1, C8 | couronne agrandie |
| Condensateur électrolytique D6,3 pas 2,5 (THT) | C6, C10 | couronne agrandie |
| LED 5 mm (THT) | D6, D7 | A = 1 sur les deux LED |
| JST-XH 2P (THT) / JST-XH 3P (THT) | J6 / J5 | couronne agrandie |
| Point de test (THT) | TP1–TP6 | couronne agrandie |

### 8.6 Reste à faire

- [x] Export `.epro2` dans `pcb/NoeudChauffageSdb/` (Fichier → Enregistrer sous → Project save as local → epro2 V3).
- [ ] Commit (procédure NAS de Pilotage_Maison, sans `global.h`).
- [ ] Commande JLCPCB (fichiers de fabrication depuis EasyEDA ; options de commande à choisir).
- [x] Tracé du boîtier imprimé (2026-10-24, § 6).
- [ ] À réception : mesures du § 7 ; cotes à vérifier avant impression du boîtier.

## 9. Pièces LCSC (références EasyEDA)

Pièces choisies au départ ; les empreintes réellement posées sont celles des fiches personnelles du § 8.5 quand elles
existent.

| Pièce | part_uuid |
|---|---|
| XY301V-A-5.0-2P | e730523cb824432a9a1d9eff97956ba1 |
| 1N5819 YFW (K = 1, A = 2) | 1ee72050fd6a4fcc98af0c234119faba |
| 25YXF47MEFC5X11 | cd7b80a500f2450da11c7d3827af885b |
| R-78E5.0-1.0 Recom | d278c4b2403c42c986a9d5d38b36bacd |
| KM101M016E110A | 0d9946191a824314ba3471ab80afdb43 |
| C320C104K5R5TA | ae110dce71644cc0a180ce765b3811e3 |
| UVR1C100MDD | 2f6c34bd2b064833b9ae0fcaed4dffac |
| B3B-XH-A(LF)(SN) | e3145416944046c8ae9e43248edd775d |
| B2B-XH-A(LF)(SN) | 4d018698282b47d4893c87aca1c32f67 |
| 2N2222A YFW (E-B-C, TO-92 pas 1,27) | ab8692c282834925ae978a85c39660aa |
| PCF14JT220R | 17b86a87c78441bbbc11fb439e6f4df2 |
| PCF14JT10K0 | f1eeb5691bc045a98708b6c70ada50f4 |
| PCF14JT22R0 | 27290ed8e0bc43fcbde73d73dc34648f |
| PCF14JT100R | d10cc727db6343edb06ca77e0d1646f5 |
| PCF14JT330R | a6bace7c685f4201a955ced70d6a223d |
| XL-502UBC (bleue) | c3ab3a2f55bc4832809069ffaafbb617 |
| XL-502SURC (rouge) | f4748221d73945ed8ad3e799566e1ea0 |

Barrettes mâles JP1, JP2 et points de test : à choisir à la pose.
