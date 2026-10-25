# Capteur C3 Xiao (variante DHT20 sur piles) — Boîtier

Version du 2026-10-25. **État : premier tracé, d'après la carte routée (`../../pcb/Capteur C3 Xiao/CapteurC3Xiao.md`,
même circuit imprimé que le Capteur S3) ; pas encore imprimé.**

Source OpenSCAD commentée : `boitier_c3.scad` (réglages dans Fenêtre > Customizer). Pièces à imprimer :
`c3_boite.stl`, `c3_couvercle.stl`, `c3_socle.stl` et `c3_trappe.stl` (PETG opaque). Aperçus : `apercu_eclate.png`,
`apercu_coupe.png`, `apercu_dessus.png`, `apercu_dessous.png`.

## 1. Besoins (décidés le 2026-10-25)

| Besoin | Réponse |
|---|---|
| Variante | **DHT20 sur piles** : 4 piles AA, interrupteur, DHT20 ; pas de flotteur (J5, R4, R5, C7 non montés), pas de LoRa |
| Pose | **posé** au-dessus d'une armoire ou sur une étagère (4 patins), pas vissé |
| Disposition | **deux pièces empilées** : boîte de la carte en haut, **socle des piles en dessous**, réunis par 4 vis M3 |
| Piles | support 4 × AA (`Composants\Alimentation\Boitier 4 x AA\Mesures.md`) vissé sous le fond de la boîte, piles vers le bas, **trappe à glissière sous le socle**, sans outil, cran, sortie par le **côté gauche** |
| Interrupteur | **MTS-102** dans la paroi basse |
| DHT20 | **dehors, sur la paroi haute** (on retourne le boîtier pour changer les piles : rien ne dépasse dessus ni dessous) |
| Antenne | Wi-Fi souple, à l'intérieur contre la paroi haute |
| Prise USB | **pas de fente** : on ouvre le couvercle pour programmer (interrupteur sur ARRÊT avant de brancher l'USB) |
| Matière | PETG opaque, parois de 2 mm |

## 2. Dimensions

| Cote | Valeur |
|---|---|
| Boîte (en haut) | intérieur 64 × 83 × 27 mm |
| Socle (en bas) | même contour, 21 mm de haut (support 16,5 + jeu 1 + trappe 2 + rebords 1,5) |
| Extérieur | 76 × 95 mm (colonnes comprises) × 52 mm de haut (couvercle compris) |
| Parois, fond, couvercle | 2 mm |

Vue de côté (posé sur l'étagère) :

```
   ┌──────────────────────────────┐ ← couvercle (4 vis M3)
   │  carte + XIAO C3             │   boîte : intérieur 64 × 83 × 27
   │  ──plots──   ▫ bossage       │
   ├══════════════════════════════┤ ← fond de la boîte (support vissé dessous)
   │  support 4 × AA, piles vers  │   socle : 21 mm
   │  le bas                      │
   └═══[ trappe → sort à gauche ]═┘ ← dessous ; patins aux 4 coins
```

Vue de dessus (couvercle retiré) :

```
              paroi haute : DHT20 dehors ─ ┌──────────┐o═ broches → câble
   ┌───────────────────────────────────────└──────────┘──┐
   │ ░░░░░░░ antenne Wi-Fi ░░░░░░                         │
   │ ┌─────────────────────────────┐                      │
   │ │ XIAO C3                [J1] │                      │
   │ │                             │                      │
   │ │      carte 46 × 65          │                      │
   │ │      (tournée de 180°)      │                      │
   │ │      [J4]     [J3]          │                      │
   │ └─────────────────────────────┘          ▯ fils piles│
   └────[MTS]─────────────────────────────────────────────┘
          ↑ interrupteur (paroi basse)
```

Coordonnées ci-dessous : X depuis la paroi gauche intérieure, Y depuis la paroi basse intérieure, Z depuis le fond
intérieur de la boîte (négatif dans le socle).

## 3. Détail

### 3.1 Carte

- Vissée sur **4 plots** Ø 7 × 4 mm (vis M3 dans le PETG, avant-trou Ø 2,5), aux trous de la carte (3,5 mm des coins).
- **Tournée de 180°** par rapport au dessin EasyEDA : borniers J3 (piles) et J4 (interrupteur) en bas, J1 (DHT20) à
  droite, XIAO en haut à gauche.
- Bord de la carte à 2 mm de la paroi gauche, 16 mm de la paroi droite, **15 mm de la paroi basse** (fils des
  borniers, interrupteur) et 3 mm de la paroi haute.
- XIAO C3 seul sur barrettes femelles : dessus **estimé à 18,6 mm** au-dessus de la carte (Z ≈ 24,2), soit 2,8 mm sous
  le couvercle.

### 3.2 Interrupteur MTS-102 (paroi basse)

- Trou Ø 6,2 à X = 14 mm (à la place de J5, non monté, loin de la fiche J4), **Z = 20 mm** : le corps (≈ 18 mm derrière
  la paroi avec les cosses) passe au-dessus des borniers ; fils vers J4.

### 3.3 DHT20 (paroi haute, dehors)

- Plaquette à plat contre la paroi haute, **capteur à gauche, broches coudées à droite**, centrée à Z = 13,5 mm
  (milieu de la boîte fermée) ; de X = 21,5 à 50 mm.
- Trou Ø 3,4 à X = 43,5 mm pour la vis M3 + écrou (à l'intérieur, à droite de l'antenne).
- **Trou du câble Ø 6** à X = 57 mm, sous la pointe des broches ; les fils vont à J1 ; reboucher le trou au montage.

### 3.4 Antenne Wi-Fi

- Contre la paroi haute, de X = 1 à 41 mm, bord bas sur un appui de 1 mm à Z = 5.

### 3.5 Support de piles (sous la boîte)

- Support 62 × 57,2 × 16,5 **vissé sous le fond** de la boîte, piles vers le bas (vers la trappe), de X = 1 à 63 et de
  Y = 14 à 71,2.
- 2 vis M3 × 6 à tête plate, mises **depuis l'intérieur du support**, à travers le fond, dans 2 **bossages** Ø 7 × 3,5
  côté carte (X = 10 et 54, Y = 42,6) ; le bossage de gauche est sous la carte (0,5 mm de jeu).
- Ses fils sortent du grand côté bas, à 6 mm du coin droit, et montent par le **trou de 8 × 8 mm du fond**
  (X = 52 à 60, Y = 3 à 11) jusqu'au bornier J3.

### 3.6 Socle et trappe

- **Socle** : cadre du même contour que la boîte, 21 mm de haut, fixé sous la boîte par **4 vis M3 × 25** passées par
  en dessous dans les colonnes des coins (lamage Ø 6,5 × 3,5 pour les têtes, avant-trous de 10 mm dans les colonnes de
  la boîte).
- **Trappe** (2 mm) sous le support, retenue par des **rebords** de 1,5 mm le long des parois basse et haute et par un
  rebord de 6 mm au bout droit ; **appuis** de 1,5 mm au-dessus (chanfreinés pour s'imprimer sans support).
- On l'ouvre en la poussant **vers la gauche** avec le pouce (5 stries creuses près du bout droit) : elle sort par une
  **fente de la paroi gauche** du socle.
- **Cran** : une bossette de 0,5 mm sous la trappe, près du bout droit, tombe dans une rainure du rebord droit.
- **Patins** autocollants sous les 4 coins du socle (ils cachent les têtes des vis).

### 3.7 Couvercle

- 4 vis **M3** dans les colonnes des coins (avant-trous Ø 2,5 sur 10 mm), passages Ø 3,4 ; rebord de centrage de 1,5 mm
  (jeu 0,3 mm). Rien dessus.

## 4. Montage

1. Imprimer la boîte, le couvercle, le socle et la trappe (PETG, remplissage ≥ 30 %). Le socle s'imprime rebords de
   la trappe sur le plateau, le couvercle face extérieure sur le plateau (les STL sont déjà orientés).
2. Monter l'interrupteur dans la paroi basse (écrou dehors), souder ses fils (vers J4).
3. Visser le support de piles sous le fond (2 vis M3 × 6, depuis l'intérieur du support), passer ses fils par le trou du
   fond.
4. Visser la carte sur les plots (4 vis M3 × 6) ; brancher les piles sur J3 (+ / −), l'interrupteur sur J4.
5. Coller l'antenne Wi-Fi contre la paroi haute, brancher son câble sur le XIAO.
6. Visser le DHT20 sur la paroi haute (vis M3 + écrou), passer son câble par le trou, brancher J1, reboucher le trou.
7. Visser le socle sous la boîte (4 vis M3 × 25), coller les patins.
8. **Interrupteur sur ARRÊT**, fermer le couvercle (4 vis M3 × 8).
9. Piles : retourner le boîtier (sur un côté), pousser la trappe vers la gauche, mettre les 4 piles, refermer jusqu'au
   cran.

## 5. À vérifier avant impression

- Hauteur réelle du XIAO C3 sur la carte montée (dessus à 18,6 mm prévu ; couvercle à 21,4 mm au-dessus de la carte).
- Sous la carte, à l'endroit du bossage gauche (X = 10, Y = 42,6, soit PCB ≈ 38 ; 37) : aucune soudure qui descende de
  plus de 0,5 mm (C7, R4, R5 ne sont pas montés dans cette variante).
- Têtes des vis du support de piles : assez plates pour ne pas gêner les piles.
- Longueur des vis du socle (M3 × 25 prévues : socle 21 − lamage 3,5 + 7 mm dans la colonne).
