# NoeudChauffageSdb — Boîtier

Version du 2026-10-24. **État : premier tracé, d'après la carte routée (`../../pcb/NoeudChauffageSdb/NoeudChauffageSdb.md`
§ 8) ; pas encore imprimé.**

Source OpenSCAD commentée : `boitier_noeud.scad` (réglages dans Fenêtre > Customizer). Pièces à imprimer :
`noeud_boite.stl` et `noeud_couvercle.stl` (PETG opaque). Aperçus : `apercu_eclate.png`, `apercu_coupe.png`,
`apercu_dessus.png`.

## 1. Besoins (décidés le 2026-10-24)

| Besoin | Réponse |
|---|---|
| Pose | dans la salle de bain, à l'abri ; à plat contre le mur, **couvercle face au chauffage** (≈ 1,80 m) |
| Fixation | 4 vis : 4 oreilles, vis à bois Ø 4 à tête fraisée |
| Prise USB-C | à gauche, fente |
| Câble 12 V | seul câble sortant, à droite : fente + serre-câble à l'intérieur |
| Micro et LED IR | **à l'intérieur**, sous le couvercle : grille devant le micro, LED IR passées dans le couvercle |
| LED témoins D6 / D7 | visibles par 2 trous du couvercle |
| Couvercle | vissé |
| Antenne Wi-Fi souple | 40 × 20 mm, câble u.FL de 52 mm (la même que le capteur C3 et le Linky), collée contre une paroi |
| Matière | PETG opaque, parois de 2 mm |

Pièces : micro MAX4466 (`Composants\Son\Max4466\Mesures.md`), LED IR **Everlight IR333C-A** 5 mm 940 nm
(`Composants\LED\IR333C-A\Mesures.md` ; les HIR204C de l'IR Box sont des 3 mm).

## 2. Dimensions

| Cote | Valeur |
|---|---|
| Intérieur | 66 × 62 × 30 mm (hauteur : du fond intérieur au dessous du couvercle) |
| Extérieur, fermé | 78 × 74 × 34 mm (colonnes des coins comprises) ; 92 mm de haut avec les oreilles |
| Parois, fond, couvercle | 2 mm |

Vue de face (posé contre le mur, couvercle vers soi) :

```
            (oreille)                         (oreille)
  ◯─────────────────────────────────────────────────◯
  │                                         [pont]   ╞═ câble 12 V
  │ ┌──────────────────────────────────────────────┐ │  (fente ouverte vers le haut)
  │ │          J5 micro   J6 LED IR      J1 12 V   │ │
  │ │                   ┌──────┐                   │ │
 ═╡ │ ┌──────┐          │micro │     (IR)   (IR)   │ │
USB │ │ XIAO │          │ grille                   │ │
  │ │ └──────┘          └──────┘        U1         │ │
  │ │                                              │ │
  │ │                  o D7              o D6      │ │
  │ └──────────────────────────────────────────────┘ │
  │      ░░░░░░░░░░ antenne 40 × 20 ░░░░░░░░░         │  paroi basse
  ◯─────────────────────────────────────────────────◯
            (oreille)                         (oreille)
```

Coordonnées ci-dessous : X depuis la paroi gauche intérieure, Y depuis la paroi basse intérieure, Z depuis le fond
intérieur.

## 3. Détail

### 3.1 Carte

- Vissée sur **4 plots** Ø 7 × 4 mm (vis M3 dans le PETG, avant-trou Ø 2,5), aux trous de la carte (3,5 mm des bords).
- Carte vue composants vers soi, **tournée de 180°** par rapport au dessin EasyEDA : XIAO et prise USB-C à gauche,
  prises J1 / J5 / J6 en haut, LED témoins en bas. Le placement de la carte n'a pas été modifié.
- Bord de la carte à 1 mm des parois gauche et droite, à 4 mm de la paroi basse (antenne) et à 10 mm de la paroi
  haute (fiches JST, câble 12 V).

### 3.2 Prise USB-C (paroi gauche)

- Fente 13 × 8 mm à coins arrondis, centrée sur la prise : Y = 23 mm, **Z = 22,1 mm** (plots 4 + carte 1,6 + 16,5 mm
  mesurés sur le XIAO monté sur barrette femelle, comme pour le Linky).

### 3.3 Câble 12 V (paroi droite)

- Le câble entre dans le bornier J1 par le haut (bornier en haut à droite de la carte), fait un coude et sort par la
  paroi droite : **fente de 5 mm, ouverte vers le haut** (le couvercle la referme), fond arrondi à 8 mm du fond
  intérieur, à Y = 57 mm.
- **Pont de serre-câble** entre la fente et J1 (X = 60,5) : bloc 6 × 9 × 7 mm, traversé sous son dessus par un passage
  3,5 × 2 mm pour un collier de 2,5 mm. Une traction sur le câble ne tire pas sur le bornier.

### 3.4 Antenne Wi-Fi

- Collée contre la **paroi basse**, de X = 13 à 53 mm, bord sur un appui de 1 mm à 7 mm du fond intérieur.
- La prise u.FL est au bout du XIAO opposé à l'USB (vers le centre) : le câble de 52 mm suffit.

### 3.5 Couvercle (face au chauffage)

- 4 vis **M3** dans des colonnes Ø 7 aux 4 coins, à l'extérieur de l'intérieur, avant-trous Ø 2,5 sur 10 mm ;
  passages Ø 3,4. Rebord de centrage de 1,5 mm sous le couvercle (jeu 0,3 mm).
- **Micro MAX4466** :
  - plaquette vissée sous le couvercle par **2 vis M2 × 6** dans 2 plots Ø 3,4 × 5,8 mm, à **10 mm d'entraxe**
    (avant-trous Ø 1,6) ;
  - capsule vers le couvercle, broches vers le haut (vers J5) ;
  - centre de la capsule à X = 37, Y = 28,7 ;
  - la capsule (5,3 mm) arrive à 0,5 mm du couvercle : **rondelle de mousse conseillée** autour de la capsule, pour que
    le son passe par la grille et non par l'intérieur du boîtier ;
  - **grille** de 7 trous Ø 1,5 (un au centre, 6 sur un cercle de 2,8 mm) face à la capsule ;
  - le dos du micro (potentiomètre de gain) est à environ 20,5 mm du fond, soit environ 3 mm au-dessus des
    condensateurs C6 / C10 et du cavalier JP2 ;
  - le gain se règle couvercle ouvert.
- **LED IR** (2 × IR333C-A) :
  - trous Ø 5,3 à X = 49 et 59, Y = 36 ;
  - collier Ø 8,5 × 3 mm sous chaque trou : on pousse la LED de l'intérieur, la collerette (Ø 5,9) bute sous le collier,
    le dôme dépasse d'environ 2,6 mm ; une goutte de colle ;
  - fils soudés sur les pattes, vers J6 ;
  - faisceau étroit (20°) : le couvercle doit regarder le chauffage.
- **LED témoins D6 (bleue) et D7 (rouge)**, soudées sur la carte :
  - trous Ø 5,3 à X = 54 (D6) et 33 (D7), Y = 10 ;
  - **tube-guide** Ø 8,5 × 8 mm sous chaque trou : la collerette de la LED bute au bas du tube, le dôme arrive à
    1,4 mm sous la face du couvercle ;
  - **à la soudure : laisser 16,4 mm de patte entre la carte et la collerette** (cale de 16 mm conseillée) ; la patte
    courte (cathode) fait 26,5 mm, il en reste assez pour traverser la carte.
- À imprimer face extérieure sur le plateau (le STL est déjà retourné). Les colliers, tubes et plots sont alors
  dessus : pas de support.

### 3.6 Fixation au mur

- 4 oreilles (2 en haut, 2 en bas, à X = 12 et 54 mm), 14 mm de large, 4 mm d'épaisseur, au niveau du fond ;
  trou Ø 4,5 à 6 mm de la paroi, **fraisé à 90°** (Ø 8,5) pour une vis à bois Ø 4 à tête fraisée. On visse le boîtier
  sans l'ouvrir.

## 4. Montage

1. Imprimer la boîte et le couvercle (PETG, remplissage ≥ 30 %).
2. Souder D6 et D7 sur la carte avec 16,4 mm de patte (cale).
3. Visser le boîtier au mur (4 vis à bois Ø 4).
4. Visser la carte sur les plots (4 vis M3 × 6).
5. Coller l'antenne contre la paroi basse (appui dessous), brancher son câble sur le XIAO.
6. Couvercle : visser le micro (2 vis M2 × 6, rondelle de mousse autour de la capsule), passer et coller les 2 LED IR.
7. Brancher les fiches du micro (J5) et des LED IR (J6).
8. Poser le câble 12 V dans la fente, sur le pont, le serrer avec le collier, le visser dans J1 (+ à droite).
9. Brancher le câble USB-C si besoin, fermer le couvercle (4 vis M3 × 8) en guidant D6 / D7 dans leurs tubes.

## 5. À vérifier avant impression

- Hauteur réelle de la prise USB-C du XIAO sur la carte montée (22,1 mm prévu).
- Diamètre du câble 12 V (fente de 5 mm prévue).
- Hauteur réelle des composants sous le micro (C6, C10, JP2 avec cavalier : ≈ 12 mm prévus au-dessus de la carte).
- Diamètre de la capsule du micro (Ø 10 mesuré ; les plots Ø 3,4 la frôlent à 0,1 mm près).
- Fiche du câble USB-C utilisé (fente 13 × 8).
