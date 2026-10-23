# Module Linky — Schéma global

Version du 2026-10-24. **État : schéma saisi dans EasyEDA (projet en ligne « Module Linky »), circuit imprimé placé,
routé (DRC sans erreur) et sérigraphié (§ 7) ; pas encore fabriqué.**

Passage sur circuit imprimé du lecteur de téléinformation (TIC) du compteur Linky, qui fonctionne aujourd'hui sur
platine d'essai, puis boîtier imprimé vissé à côté du compteur. Besoin, logiciel et mise au point :
`U:\Info-Developpement\Pilotage_Maison\HA\Linky\Linky_Rapport_Technique.md`. Méthode : celle de la borne Squash et
du contrôleur HA (composants réels à pattes nommées, tableau validé avant pose, éléments hors carte non dessinés).

## 1. Rôle et code

| Carte | Rôle | Code |
|---|---|---|
| **XIAO ESP32-C3** | lit la sortie TIC du Linky (mode historique, 1 200 bauds, 7E1) et remonte PAPP, BASE, PTEC, ADCO à Home Assistant par Wi-Fi | ESPHome `linky.yaml` (composant `teleinfo`, `historical_mode: true`) |

**Aucun changement de code** : la carte garde `rx_pin: GPIO20` et `logger: baud_rate: 0`.

### 1.1 Brochage du XIAO

| Fonction | GPIO | Broche du XIAO | Remarque |
|---|---|---|---|
| RX TIC (sortie de l'optocoupleur) | 20 | **D7 (RX)** | UART matériel ; **pas D9** (GPIO9 = BOOT) |
| Alimentation | – | 5V, GND | par le câble USB-C (adaptateur secteur) |
| Tirage RX | – | 3V3 | résistance 10 kΩ |

## 2. Schéma global

```
HORS CARTE        │                 SUR LE CIRCUIT IMPRIMÉ                     │ HORS CARTE
                  │                                                            │
Linky I1 ═════════╪═► J1-1 ──[R1 1 kΩ]──► U1-1 ┐                                │
(fils ≈ 30 cm)    │                            │ SFH620A (2 LED tête-bêche)     │
Linky I2 ═════════╪═► J1-2 ────────────────► U1-2 ┘                             │
                  │                     ┊ isolement optique ┊                  │
                  │      3V3 (A1) ──[R2 10 kΩ]──┬── U1-4 (collecteur)           │
                  │                             └──► A1 D7 / GPIO20 (RX)        │
                  │                   GND (A1) ──── U1-3 (émetteur)             │
                  │                                                            │
adaptateur USB ═══╪═► A1 XIAO ESP32-C3 (prise USB-C au bord de la carte)        │
                  │      u.FL ══════════════════════════════════════════════════╪══► antenne Wi-Fi souple
                  │                                                            │   (collée dans le boîtier)
                  │      5V (A1) ──[R3 1 kΩ]── LED_A ─► J2 ═════════════════════╪══► LED « allumé »
                  │                                                            │   (couvercle)
```

Deux masses **séparées** : côté compteur (I1, I2) et côté XIAO. Seul l'optocoupleur les relie, optiquement.

## 3. Détail par fonction

### 3.1 Entrée TIC

- **J1** : bornier à vis 2 points type **KF301-2P** (en stock, bleu, pas de 5,0 mm, corps 10 × 8 × 10 mm), `I1` et `I2`, au bord de la carte, face au passage des fils.
  Liaison au compteur par fils d'environ 30 cm (paire torsadée conseillée, contre les erreurs CRC).
- Sortie TIC **non polarisée** : I1 et I2 peuvent être inversés (l'optocoupleur a deux LED tête-bêche).
- La borne **A** du Linky (alimentation, ≈ 130 mW) n'est **pas** utilisée : insuffisante pour le Wi-Fi.

### 3.2 Optocoupleur

- **U1** : **SFH620A-1** (celui de la platine d'essai, gain CTR 40 à 125 %) en boîtier DIP-4 traversant, sur support
  DIP-4 (remplaçable). Brochage : 1 et 2 = LED (entrée alternative), 3 = émetteur, 4 = collecteur.
- **R1** (1 kΩ) en série entre J1-1 et U1-1 : limite le courant dans les LED.
- **R2** (10 kΩ) entre U1-4 et le 3V3 du XIAO : tirage de la sortie (porteuse présente → 0 V, absente → 3,3 V).
- Le SFH620A redresse la porteuse 50 kHz (deux LED) et la lisse (phototransistor trop lent pour la suivre) : l'UART
  reçoit directement l'enveloppe à 1 200 bauds. Le PC817 ne convient pas : une seule LED, il ne redresse qu'une
  alternance sur deux (son gain n'est pas en cause), voir le rapport § 5.
- Écart d'isolement : pistes côté compteur (J1, R1, U1-1/2) et côté XIAO (U1-3/4) tenues à au moins 2 mm l'une de
  l'autre.

### 3.3 Carte XIAO ESP32-C3

- **A1** : XIAO ESP32-C3 sur **2 barrettes femelles 1 × 7** (démontable, comme la borne Squash).
- Alimenté par sa **prise USB-C** (adaptateur secteur 5 V), qui sert aussi à le reflasher : la prise est au bord de la
  carte, face à une fente du boîtier. Pas d'autre alimentation sur la carte, donc ni diode ni convertisseur.
- Boutons B (BOOT) et R (RESET) du XIAO : accessibles en ouvrant le couvercle (rarement utiles).

### 3.4 Voyant « allumé »

- LED 5 mm verte sur le couvercle, reliée par **J2** (JST-XH 2), comme sur le contrôleur HA.
- **R3** (1 kΩ) sur la carte, alimentée par la broche **5V** du XIAO : la LED est allumée quand l'USB est alimenté.
  Courant ≈ 3 mA.

### 3.5 Antenne Wi-Fi

- Antenne **souple rectangulaire** (celle des essais : −45 dBm près du compteur) sur la prise u.FL du XIAO (au bout
  opposé à l'USB), collée à l'intérieur du boîtier, loin du bornier.

## 4. Connecteurs

| Repère | Type | Broches | Vers |
|---|---|---|---|
| J1 | Bornier à vis 2 points KF301-2P, pas de 5,0 mm | 1 `I1`, 2 `I2` | sortie TIC du Linky |
| J2 | JST-XH 2 | 1 `LED_A` (anode), 2 `GND` (cathode) | LED « allumé » |
| (A1) | USB-C du XIAO | – | adaptateur secteur 5 V |

## 5. Nomenclature

Liste d'achat détaillée (références, pièces non dessinées, hors carte) : `Linky_Composants.md` ; composants dessinés :
`Linky_BOM.xlsx`.

| Repère | Composant | Qté |
|---|---|---|
| A1 | XIAO ESP32-C3, sur 2 barrettes femelles 1 × 7 | 1 |
| U1 | Optocoupleur SFH620A-1, DIP-4, sur support DIP-4 | 1 |
| R1 | Résistance 1 kΩ 1/4 W | 1 |
| R2 | Résistance 10 kΩ 1/4 W | 1 |
| R3 | Résistance 1 kΩ 1/4 W | 1 |
| J1 | Bornier à vis 2 points KF301-2P, pas de 5,0 mm (en stock) | 1 |
| J2 | Embase JST-XH 2 broches | 1 |
| – | Hors carte : adaptateur USB 5 V + câble USB-C, antenne Wi-Fi souple u.FL, LED 5 mm verte, fils I1/I2 (≈ 30 cm) | – |

Fiches EasyEDA réutilisables : XIAO ESP32-C3 (carte Capteur C3 Xiao), résistances et JST-XH (borne Squash,
contrôleur HA). À chercher : SFH620A (DIP-4), bornier KF301-2P (pas de 5,0 mm).

## 6. Vérifications avant la saisie

1. ~~Référence de l'optocoupleur~~ : **SFH620A-1**, DIP-4 traversant (2026-10-24).
2. ~~LED~~ : sur le couvercle, reliée par J2 (2026-10-24).
3. ~~Fixation et forme~~ : boîtier vissé à côté du compteur ; **fils I1/I2 et câble USB aux deux bouts opposés** :
   J1 au bord d'un petit côté de la carte, prise USB-C du XIAO au bord du petit côté opposé (2026-10-24).
   La prise u.FL du XIAO se trouve alors du côté du bornier : le câble de l'antenne l'emmène loin des fils TIC.
4. ~~Bornier~~ : KF301-2P en stock, pas de 5,0 mm, corps 10 × 8 × 10 mm
   (`U:\Info-Developpement\Composants\Connectique\Borniers\`) ; pas mesuré au pied à coulisse : **5,00 mm** (2026-10-24).

## 7. Circuit imprimé (état au 2026-10-24 : placé, routé, sérigraphie complète)

Projet EasyEDA Pro **en ligne** « Module Linky » (schéma « Module Linky », page « Carte »), copie exportée
`Linky_Projet_2026-10-24.epro2` avec les vues `Linky_2D/3D/PCB_2026-10-24.png` et le schéma
`Linky_Schemas_2026-10-24.zip`. Fabrication prévue comme la borne Squash et le contrôleur HA (JLCPCB, 2 couches,
1,6 mm, HASL, tout traversant soudé à la main).

### 7.1 Carte et placement

Carte de **58 × 33 mm**, origine au coin bas gauche. 4 trous M3 (Ø 3,2, non métallisés) aux coins, à 3,5 mm des bords.

```
 ┌──────────────────────────────────────────────────────────┐
 │ (M3)              Module Linky                     (M3)  │
 │       R1 [ R1 1 kΩ ]                 A1                  │
 │ ┌───┐                ┃R2   ┌────────────────────────┐    │
 │ │J1 │ I1    ┌──U1──┐ ┃10k  │ · · · · · · ·          ├────┤
 │ │   │ I2    └──────┘ ┃     │    A1 XIAO ESP32-C3    │USB-C → câble USB
 │ └───┘                      │ RX · · · 3V3 GND 5V    ├────┤
 │ J1 TIC   J2 LED            └────────────────────────┘    │
 │ (M3)     [J2][ R3 1 kΩ ]        XIAO ESP32-C3      (M3)  │
 └──────────────────────────────────────────────────────────┘
   ← fils I1/I2 du Linky
```

| Élément | Position |
|---|---|
| A1 XIAO | bord droit, plaquette au ras du bord, prise USB-C qui dépasse de 1,5 mm (fente du boîtier) ; broches utilisées (D7/RX, 3V3, GND, 5V) sur la rangée du bas |
| J1 bornier | bord gauche, entrée des fils vers l'extérieur (vérifié en 3D), I1 en haut |
| U1 | centre gauche, entrées (1, 2) côté J1, sorties (3, 4) côté XIAO |
| R1 | au-dessus de U1, entre J1-1 et U1-1 |
| R2 | debout entre U1 et le XIAO |
| J2, R3 | en bas à gauche |

Modèles 3D : XIAO « Seeed Studio XIAO-ESP32-C3 » (réglage des autres cartes), modèles LCSC pour les autres
(celui de R1 reporté sur R2, dont la fiche n'en a pas).

### 7.2 Routage

Routage automatique (Copilot) sans plan de masse, puis plans de masse créés à la main : **DRC sans erreur**.

| Règle | Valeur |
|---|---|
| 5V, 3V3, GND | pistes 0,6 mm (min. 0,5) |
| Signaux (I1, I1_R, I2, RX_TIC, LED_A) | pistes 0,4 mm (min. 0,3) |
| GND | plans sur les deux faces (`GND_TOP`, `GND_BOTTOM`), connexions à rayons thermiques ; aucun via |
| Écarts | 0,3 mm ; 0,5 mm au bord |
| Isolement | **pas de cuivre de masse côté compteur** : les plans s'arrêtent à x = 20 mm au-dessus de y = 11,5 mm (J1, R1, entrées de U1 + 2 mm) ; au moins 7,6 mm entre une pastille côté compteur et une pastille côté XIAO |

### 7.3 Sérigraphie

Repères hors des contours, 1,1 mm, droits (0°). Textes : « Module Linky » (1,5 mm), « XIAO ESP32-C3 » sous le module,
« J1 TIC » + I1 / I2 face aux vis, « J2 LED » + `+` (broche 1, LED_A) / `-` (broche 2, GND).

**Suite** : fichiers de fabrication (Gerber) et commande ; boîtier imprimé (vissé à côté du compteur).
