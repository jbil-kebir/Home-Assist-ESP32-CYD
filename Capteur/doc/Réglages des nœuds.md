# Réglages des nœuds capteurs (page Web)

Valeurs à saisir sur la page Web de chaque nœud, à partir de Capteur 3.9 (délai d'alerte). Décidé le 15/11/2026.

## Principe

- **Forçage** (« Intervalle forçage remontée ») : la mesure part au moins une fois par forçage, même inchangée. Entre deux
  forçages, une mesure part dès qu'elle change assez (0,2 °C, 5 % d'humidité, 0,05 V).
- **Retard toléré** : importance du capteur. Le nœud calcule lui-même le **délai d'alerte** : forçage arrondi au réveil
  suivant (chaque réveil compté sommeil + 10 s) + retard toléré + 60 s de marge. Au-delà, le CYD affiche WDG sur le
  capteur, et Home Assistant envoie un mail.
- La colonne « Délai d'alerte » est ce que la page du nœud doit afficher sur la ligne
  « → alerte si aucune nouvelle pendant … », et ce que la page du CYD doit afficher pour ce capteur
  (« annoncé par le nœud »).
- Batteries : retard plus long (7200 s) pour qu'un nœud en panne ne déclenche d'abord qu'un mail (thermomètre ou
  flotteur) ; le bandeau du CYD passe à l'orange quand la batterie est à son tour en retard.

## Tableau

| Nœud | Deep sleep | Capteur (préfixe) | Forçage | Retard toléré | Délai d'alerte |
|---|---|---|---|---|---|
| **ThChRDC** | coché, 900 s | Thermomètre (thhum_) | 3600 s | 3600 s | 2 h 02 |
| | | Batterie (bat_) | 3600 s | 7200 s | 3 h 02 |
| **ThCh1er** | coché, 900 s | Thermomètre (thhum_) | 3600 s | 3600 s | 2 h 02 |
| | | Batterie (bat_) | 3600 s | 7200 s | 3 h 02 |
| **ThCave** | coché, 900 s | Thermomètre (thhum_) | 3600 s | 3600 s | 2 h 02 |
| | | Batterie (bat_) | 3600 s | 7200 s | 3 h 02 |
| | | Jour / nuit (jn_) | 3600 s | 3600 s | 2 h 02 |
| **ThRemise** | coché, 900 s | Thermomètre (thhum_) | 3600 s | 3600 s | 2 h 02 |
| | | Batterie (bat_) | 3600 s | 7200 s | 3 h 02 |
| | | **Flotteur (mfv_)** | 3600 s | **0 s** | **1 h 02** |
| **ThSdb** | coché, **600 s** | **Thermomètre (thhum_)** | 3600 s | **0 s** | **1 h 02** |
| | | **Voyant chaudière (rgb_)** | 3600 s | **0 s** | **1 h 02** |
| | | Batterie (bat_) | 3600 s | 7200 s | 3 h 02 |

## Raisons

- **ThChRDC, ThCh1er, ThCave** : non critiques, alerte au bout de 2 h. Sommeil 900 s : 4 réveils pour atteindre le
  forçage (4 × 910 = 3640 s) + 60 s + 3600 s = 7300 s.
- **ThRemise** : le flotteur réveille le nœud dès que le niveau change ; si le nœud tombe en panne, le flotteur alerte
  au bout d'1 h, avant le thermomètre.
- **ThSdb** : nœud de la chaudière, sommeil 600 s (6 réveils : 6 × 610 = 3660 s + 60 s = 3720 s). Chaudière allumée,
  le nœud reste éveillé et envoie plus souvent : le délai reste juste (jamais de fausse alerte). Alimenté par USB :
  batterie secondaire.
- **PrGuirlande, NoeudChauffageSdb** : non concernés (pas de deep sleep, pas de capteur à délai d'alerte : le nœud
  chauffage disparaît normalement quand son relais coupe ; la guirlande garde sa présence par ALIVE).
- Autres champs (intervalle de mesure, éveil maximum, watchdog ALIVE) : inchangés.

## Saisie

1. Flasher le nœud avec son profil (`env:<Nom>_Xiao_C3` ou `env:<Nom>_Xiao_S3`, voir `platformio.ini`).
2. Nœud éveillé (Home Assistant : décocher « Deep sleep actif », Enregistrer, puis RESET du nœud) : saisir forçages et
   retards tolérés sur la page Web, vérifier les lignes « → alerte si … ».
3. Régler le sommeil et cocher le deep sleep **en dernier**, enregistrer.
4. Nœuds LoRa (ThRemise, ThCave) : pas de page Web ; forçages et retards tolérés par Home Assistant (carte Réglages
   du nœud, champs « Forçage … » et « Retard toléré … », Enregistrer) ou par la commande
   `SET thhum_force 3600 thhum_retard 3600 bat_force 3600 bat_retard 7200 …` (forçage : 60 à 86400 s ; retard : 0 à
   604800 s). Réponse CONFIG trop longue pour une trame LoRa : envoyée en deux trames (CONFIG puis CONFIGSUITE),
   Home Assistant les réunit ; la page du relais n'en montre que la 1re partie.
5. Contrôle : page du CYD principal, « Délai d'alerte : … (annoncé par le nœud) » pour chaque capteur.
