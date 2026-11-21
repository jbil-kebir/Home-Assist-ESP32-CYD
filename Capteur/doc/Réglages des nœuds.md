# Réglages des nœuds capteurs (page Web)

Valeurs à saisir sur la page Web de chaque nœud. Décidé le 15/11/2026 ; revu le 21/11/2026 : délai d'alerte saisi
directement, en minutes (Capteur 4.4, à la place du « retard toléré »), 180 min partout sauf flotteur et voyant
chaudière à 0 (au plus tôt).

## Principe

- **Forçage** (« Intervalle forçage remontée ») : la mesure part au moins une fois par forçage, même inchangée. Entre deux
  forçages, une mesure part dès qu'elle change assez (0,2 °C, 5 % d'humidité, 0,05 V).
- **Délai d'alerte** (minutes, Capteur 4.4) : durée sans nouvelles au-delà de laquelle le CYD affiche WDG sur le capteur
  et Home Assistant envoie un mail. Le nœud le relève à son **minimum** s'il est plus court : forçage arrondi au réveil
  suivant (chaque réveil compté sommeil + 10 s) + 60 s de marge. **0 = au plus tôt** (le minimum).
- Avant la 4.4 : « retard toléré » (s) ajouté au minimum. Converti au premier démarrage en 4.4 : retard 0 → 0, sinon
  (retard + forçage) / 60 (7200 + 3600 → 180 min) : rien à ressaisir après le flash.
- La colonne « Délai d'alerte » est ce que la page du nœud doit afficher sur la ligne
  « → alerte si aucune nouvelle pendant … », et ce que la page du CYD doit afficher pour ce capteur
  (« annoncé par le nœud »).
- 180 min pour tous les capteurs, sauf le flotteur de ThRemise et le voyant de la chaudière (ThSdb) : à 0,
  une seule trame forcée perdue suffisait à déclencher un mail (fausses alertes BatCh1er des 18, 19 et 20/11/2026 ;
  la batterie ne change presque jamais et n'envoie qu'au forçage, une trame par heure).

## Tableau

| Nœud | Deep sleep | Capteur (préfixe) | Forçage | Délai d'alerte saisi | Alerte effective |
|---|---|---|---|---|---|
| **ThChRDC** | coché, 900 s | Thermomètre (thhum_) | 3600 s | 180 min | 3 h 00 |
| | | Batterie (bat_) | 3600 s | 180 min | 3 h 00 |
| **ThCh1er** | coché, 900 s | Thermomètre (thhum_) | 3600 s | 180 min | 3 h 00 |
| | | Batterie (bat_) | 3600 s | 180 min | 3 h 00 |
| **ThCave** | coché, 900 s | Thermomètre (thhum_) | 3600 s | 180 min | 3 h 00 |
| | | Batterie (bat_) | 3600 s | 180 min | 3 h 00 |
| | | Jour / nuit (jn_) | 3600 s | 180 min | 3 h 00 |
| **ThRemise** | coché, 900 s | Thermomètre (thhum_) | 3600 s | 180 min | 3 h 00 |
| | | Batterie (bat_) | 3600 s | 180 min | 3 h 00 |
| | | **Flotteur (mfv_)** | 3600 s | **0** (au plus tôt) | **1 h 02** |
| **ThSdb** | coché, **600 s** | Thermomètre (thhum_) | 3600 s | 180 min | 3 h 00 |
| | | **Voyant chaudière (rgb_)** | 3600 s | **0** (au plus tôt) | **1 h 02** |
| | | Batterie (bat_) | 3600 s | 180 min | 3 h 00 |

## Raisons

- **ThChRDC, ThCh1er, ThCave** : non critiques, alerte au bout de 3 h, quel que soit le sommeil en cours (minimum :
  1 h 02 en sommeil 900 s, 4 × 910 + 60 = 3700 s ; 1 h 06 pour ThCh1er en sommeil 120 s, 30 × 130 + 60 = 3960 s).
- **ThRemise** : le flotteur réveille le nœud dès que le niveau change ; si le nœud tombe en panne, le flotteur alerte
  au bout d'1 h, bien avant le thermomètre.
- **ThSdb** : nœud de la chaudière, sommeil 600 s (6 réveils : 6 × 610 = 3660 s + 60 s = 3720 s) ; le voyant (au plus tôt)
  alerte au bout d'1 h, avant le thermomètre et la batterie. Chaudière allumée,
  le nœud reste éveillé et envoie plus souvent : le délai reste juste (jamais de fausse alerte). Alimenté par USB :
  batterie secondaire.
- **PrGuirlande, NoeudChauffageSdb** : non concernés (pas de deep sleep, pas de capteur à délai d'alerte : le nœud
  chauffage disparaît normalement quand son relais coupe ; la guirlande garde sa présence par ALIVE).
- Autres champs (intervalle de mesure, éveil maximum, watchdog ALIVE) : inchangés.

## Saisie

1. Flasher le nœud avec son profil (`env:<Nom>_Xiao_C3` ou `env:<Nom>_Xiao_S3`, voir `platformio.ini`).
2. Nœud éveillé (Home Assistant : décocher « Deep sleep actif », Enregistrer, puis RESET du nœud) : saisir forçages et
   délais d'alerte sur la page Web, vérifier les lignes « → alerte après … ».
3. Régler le sommeil et cocher le deep sleep **en dernier**, enregistrer.
4. Nœuds LoRa (ThRemise, ThCave) : pas de page Web ; forçages et délais d'alerte par Home Assistant (carte Réglages
   du nœud, champs « Forçage … » et « Délai d'alerte … », Enregistrer) ou par la commande
   `SET thhum_force 3600 thhum_alerte 180 bat_force 3600 bat_alerte 180 …` (forçage : 60 à 86400 s ; délai : 0 à
   10080 min). Réponse CONFIG trop longue pour une trame LoRa : envoyée en deux trames (CONFIG puis CONFIGSUITE),
   Home Assistant les réunit ; la page du relais n'en montre que la 1re partie.
5. Contrôle : page du CYD principal, « Délai d'alerte : … (annoncé par le nœud) » pour chaque capteur.
