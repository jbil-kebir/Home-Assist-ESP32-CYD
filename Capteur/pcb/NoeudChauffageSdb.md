# NoeudChauffageSdb — Circuit imprimé (proposition de schéma)

Version du 2026-10-18. Dossier de conception VALIDÉ, à reprendre au moment de dessiner la carte dans EasyEDA Pro.
**État : rien n'est encore créé dans EasyEDA.**

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
| LED IR | les mêmes que l'IR Box : 2 × HIR204C en série |
| Transistor IR | 2N2222A |
| LED témoins | sur la carte, visibles à travers le couvercle transparent ; LED bleue gardée |
| Boîtier | Hammond 1554C2GYCL (IP66, couvercle transparent) ; entrées par presse-étoupes |
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
| J6 | JST-XH 2 broches « vers LED IR (2 × HIR204C) » | B2B-XH-A(LF)(SN) | 1 → `IR_A` ; 2 → `IR_K` |
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
2 × HIR204C en série.

## 5. Dimensionnement

### 5.1 Alimentation

- D1 : 0,25 A côté 12 V → 1N5819 (1 A) suffit.
- U1 : R-78E5.0-1.0 (1 A) ≈ 2 × la pointe estimée ; un régulateur linéaire 7805 dissiperait (12 − 5) × 0,5 = 3,5 W :
  exclu.
- DA : même rôle que sur la borne Squash (XIAO alimenté par sa broche 5V, USB branché pour le reflashage).

### 5.2 Émission IR

HIR204C : 100 mA en continu ; le 1 A de la fiche technique ne vaut que pour des impulsions ≤ 100 µs et ≤ 1 % du temps,
ce que n'est pas le NEC (porteuse 38 kHz allumée environ un tiers du temps pendant les rafales). Le montage d'essai
actuel fonctionne à 1,80 m avec environ 15 mA (6 V, 197 Ω en série, 150 Ω en base, 2N3511). Cible : **100 mA en crête**
(≈ 6 fois plus de marge de portée, dans la limite du continu des LED).

| Repère | Calcul | Valeur | IR Box ESP32 1.0 |
|---|---|---|---|
| R3 (série LED) | (5 − 2 × 1,35 − 0,15) / 0,1 A = 21,5 Ω | 22 Ω 1/4 W (≈ 98 mA) | 5 Ω (≈ 0,35 A) |
| R1 (base) | courant de base = 100 mA / 10 = 10 mA ; (3,3 − 0,8) / 10 mA = 250 Ω | 220 Ω (≈ 11 mA) | 197 Ω |
| R2 (rappel) | LED IR éteinte tant que la broche n'est pas pilotée | 10 kΩ | absente |
| R4 (LED bleue) | (3,3 − ≈ 2,9) / 4 mA | 100 Ω (2 à 5 mA) | — |
| R5 (LED rouge) | (3,3 − 1,9) / 4 mA | 330 Ω (≈ 4 mA) | — |

Sécurité : si la broche IR restait bloquée à l'état haut, les LED recevraient 100 mA en continu (leur limite, pas une
surcharge) et R3 dissiperait 0,21 W (< 0,25 W). Avec les 5 Ω de l'IR Box, ce serait 0,35 A en continu dans les LED.

Pour aller plus loin si la portée manque : 15 Ω → ≈ 145 mA en crête (encore raisonnable, en 1/2 W). Les HIR204C
émettent à 850 nm alors que les récepteurs sont plutôt réglés vers 940 nm : une LED 940 nm (TSAL6200 par exemple)
serait plus efficace, sans toucher au PCB (LED sur fil).

Défaut de l'IR Box à NE PAS reprendre : LED L1 + 100 Ω entre la base et la masse (vole le courant de base, ne s'allume
presque pas). Ici la LED témoin a sa propre broche.

## 6. Boîtier et mécanique

**Hammond 1554C2GYCL** : polycarbonate, couvercle transparent, IP66 ; extérieur 120 × 65 × 40 mm ; intérieur
≈ 113,4 × 58,4 mm, hauteur 34,5 mm ; inserts laiton M3 pour fixer le PCB ; 4 vis de couvercle M4 et joint fournis.
Plan : `https://www.hammfg.com/files/parts/pdf/1554C2GYCL.pdf`.

- Entraxe exact des inserts M3 : **à lire sur le plan** (peut-être 95 × 40,5 mm) ou à mesurer sur la pièce → fixe le
  contour et les trous du PCB.
- Hauteur : composant le plus haut ≈ 16 mm (XIAO sur barrettes), largement sous les 34,5 mm.
- 3 presse-étoupes PG7 (câble 12 V, micro, LED IR) ; le micro et les LED IR sont DEHORS (enfermé, le micro entendrait mal ;
  le polycarbonate gris arrête l'infrarouge).
- Prises sur la carte (J1, J5, J6) du côté des presse-étoupes ; LED témoins visibles par le couvercle.

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

## 8. À faire au moment de la conception

- [ ] Fiche perso **TO-92 au pas de 2,54 mm** (pattes écartées) pour Q1 : l'empreinte LCSC du 2N2222A est au pas de
      1,27 mm, serrée pour la soudure à la main (à valider).
- [ ] Fiche perso **bornier XY301V agrandi** (couronne 0,40 → ≥ 0,5 mm).
- [ ] Contrôle des couronnes de toutes les empreintes à la pose (notice du modèle, § 6).
- [ ] Entraxe des inserts Hammond (§ 6).
- [ ] Projet EasyEDA en Online Mode, règles « THT soudure main JLCPCB », classes PUISSANCE (`V12_J`, `V12`, `5V_REG`,
      `+5V`, `5V_CARTE`, `GND`, `IR_V`, `IR_A`, `IR_K`) et ALIM_3V3 (`3V3`), connecteurs « vers … ».

## 9. Pièces LCSC (références EasyEDA)

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
