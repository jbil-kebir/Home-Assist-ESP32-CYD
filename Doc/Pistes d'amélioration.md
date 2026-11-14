# Pilotage_Maison — Pistes d'amélioration

Améliorations possibles, **non urgentes** : rien ne fonctionne mal sans elles. On les note ici pour ne pas les perdre et
on les reprend quand on retouche la partie concernée. Une piste réalisée est retirée de cette liste (et décrite dans le
`Version.txt` du projet concerné).

Créé le 2026-11-14. Ancienne liste du Capteur, en partie périmée : `Capteur/doc/A faire.txt`.

## 1. Capteur (nœuds Wi-Fi / LoRa)

### 1.1 Pont de mesure des piles toujours alimenté

- **Constat** (2026-10-26, ThCh1er sur piles) : le pont 220 kΩ + 100 kΩ qui mesure la tension des piles est branché en
  permanence sur les piles ; il consomme ≈ **13 µA**, y compris en deep sleep, sur ≈ 47 µA de sommeil mesurés.
- **Poids réel** : faible. Un nœud éveillé ≈ 2,4 s toutes les 10 min à ≈ 80 mA consomme en moyenne ≈ 0,3 mA ; le pont
  en représente ≈ **4 %** (quelques semaines gagnées sur une durée de vie de plusieurs mois sur 4 AA).
- **Pistes** :
  - (a) pont de plus forte valeur (par exemple 1 MΩ / 470 kΩ) avec un condensateur sur l'entrée de mesure (l'ADC a besoin
    d'une source peu résistive au moment de la mesure) ; recalculer le calibre ;
  - (b) pont coupé par un transistor (en traversant : MOSFET canal P ou PNP sur le haut du pont, commandé par une GPIO
    via un NPN), alimenté seulement pendant la mesure.
- **Quand** : à la prochaine révision des cartes à piles (Capteur S3, Capteur C3 Xiao).

### 1.2 Mémoire RTC (RTC_DATA_ATTR)

- **Constat** : certaines valeurs sont déjà gardées en mémoire RTC pendant le deep sleep (Diagnostic, MyDateTime,
  MyJourNuit, MyTor, MyWifi), mais l'audit complet n'a pas été fait.
- **Piste** : passer en revue les valeurs relues ou réécrites à chaque réveil (NVS, dernières mesures envoyées) et
  garder en RTC celles qui le permettent : moins d'écritures en NVS (usure de la flash) et moins d'émissions inutiles.

## 2. CYD (contrôleur Home-Assist)

### 2.1 Indicateur d'éveil des nœuds : option « présence »

- **Fait** : bandeau vert / rouge / gris derrière le nom de chaque nœud (état instantané : éveillé, endormi, inconnu).
- **Piste** : une option « présence » qui distinguerait un nœud vu récemment d'un nœud muet depuis longtemps.

## 3. Cartes et boîtiers

(Aucune piste pour l'instant ; voir 1.1 pour les cartes à piles.)
