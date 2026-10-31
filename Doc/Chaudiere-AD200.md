# Chaudière — Commande 433 MHz par le CYD à l'image du thermostat AD200

Version du 2026-10-31. Synthèse des essais de la nuit du 30 au 31/10/2026 : relevé du protocole radio du thermostat
d'origine, défauts trouvés dans le CYD principal et corrections (commit 7032726, Home-Assist CYD 3.4, Capteur 3.3).

## 1. Le montage

| Élément | Rôle |
|---|---|
| Chaudière gaz De Dietrich **Naema Micro 25** | régule elle-même la température de l'eau ; démarre le brûleur quand la chauffe est demandée |
| Thermostat **AD200** (De Dietrich, réf. 88017018) | thermostat d'ambiance programmable sans fil, piles ; mode **chronoproportionnel** (interrupteur 2 sur CHRONO) |
| Récepteur mural de l'AD200 | reçoit les trames 433 MHz ; **une seule sortie : un contact inverseur** (16 A) sur l'entrée thermostat de la chaudière |
| **CYD principal** (192.168.1.94) | remplace l'AD200 : émet les mêmes trames avec son CC1101, sur commande du bouton, de la page Web ou de HA (MQTT) |
| **ThSdb / CoulSdb** (192.168.1.175) | capteur de couleur TCS34725 posé sur la **DEL rouge de la chaudière**, qui suit exactement le **brûleur** ; trame MQTT `CoulSdb Del … 0/1` |

Outils d'essai : enregistreur MQTT (abonné à `home/#`) et récepteur radio **RTL-SDR** (Nooelec NESDR SMArt v5) avec
**rtl_433** 25.12, installé dans `D:\Outils\rtl_433`. Captures conservées dans `D:\Outils\rtl_433\captures`.

## 2. Protocole radio de l'AD200

### 2.1 Trames

Modulation OOK, codage par largeur d'impulsion (impulsion courte = 1, longue = 0, notation rtl_433). Les quatre trames
utiles sont **déjà celles du CYD** (`Home-Assist/src/MyChaudiere.cpp`) :

| Trame | Données (rtl_433) | Tableau du CYD |
|---|---|---|
| Commande ON | `{74}ffff5fee76fffb7fb78` | `pulseBufferON` (identique) |
| Maintien ON | `{74}ffff5fee76fdf6ff6fc` | `pulseBufferACK_ON` (dernière impulsion longue au CYD, courte à l'AD200) |
| Commande OFF | `{75}ffff5fee76fffb7fdfc` | `pulseBufferOFF` (identique) |
| Maintien OFF | `{74}ffff5fee76fdf6ffbf4` | `pulseBufferACK_OFF` (même nuance) |
| Initialisation | `{73}ffff5fee76fffbe7f28`, 5 fois à 0,24 s | aucun : émise par l'AD200 à la mise sous piles, inutile au CYD |

Durées d'impulsions : AD200 172 / 380 µs (plus deux de 584 µs) ; CYD 210 / 449 µs. Le récepteur accepte les deux.

### 2.2 Rythme

Identique pour ON et OFF : la **commande**, puis des trames de **maintien** à

| +6,5 s | +14,5 s | +55,5 s | +85,5 s | puis toutes les **625,5 s** |
|---|---|---|---|---|

### 2.3 Fréquence

Mesurée dans une fenêtre de 1 MHz (`-f 434.0M -s 1024k`) : **AD200 ≈ 434,30 MHz** ; CYD réglé à 434,092 MHz ≈ 434,30 MHz.
Le CC1101 du principal émet donc environ **+210 kHz** au-dessus de sa fréquence de réglage.

Piège : les premières mesures, faites dans une fenêtre de 250 kHz centrée sur 433,92 MHz, donnaient 433,985 et
433,931 MHz. Elles étaient **fausses** (repliement d'un signal situé hors de la fenêtre). Toujours mesurer en fenêtre large.

### 2.4 Comportement du récepteur

- Il obéit à **la dernière trame reçue**, quel que soit l'émetteur : un maintien OFF de l'AD200 a arrêté le brûleur
  commandé ON par le CYD (6 s), le maintien ON suivant du CYD l'a relancé (4 s).
- Une trame de **maintien suffit** à (re)mettre l'état : une trame perdue est rattrapée au maintien suivant.
- Il garde l'état **plus de 10 min** entre deux maintiens (38 min observées avec l'AD200 seul).
- Le brûleur suit la commande en **4 à 10 s** (délai de la mesure de ThSdb compris).
- Il accepte une large plage de fréquence : le CYD fonctionnait déjà à 434,038 MHz de réglage.

## 3. Mesures

### 3.1 Avant correction (CYD, keep-alive = commande répétée)

Durée de marche du brûleur après une **seule** trame ON (keep-alive coupé) :

| Essai | Repos avant | Durée de marche |
|---|---|---|
| 1 (trame OFF 6 s avant) | 3 min 12 | ≈ 1 min |
| 2 | 1 min 29 | ≈ 8 min 50 |
| 3 | 5 min 38 | ≈ 4 min 35 |
| Avec keep-alive (table 30, 100, 200, 400, 500 s) | 28 min | ≈ 4 min 38, malgré les trames ON |

### 3.2 AD200 seul (consigne élevée, puis 6 °C)

Commande ON à 00:46:04, brûleur allumé à 00:46:08 et **continu pendant 41 min** avec des maintiens toutes les 625,5 s ;
commande OFF à 01:27:12, brûleur éteint à 01:27:22.

### 3.3 Après correction (CYD seul)

Commande ON puis maintiens à +7, +15, +56, +86, +626 s : **brûleur continu 13 min** (70 mesures de la DEL à 1), jusqu'à
la fin de l'essai.

### 3.4 Capteur de la DEL (ThSdb)

| | Rouge | Intensité brute |
|---|---|---|
| Brûleur éteint | 75 – 76 | 279 – 280 |
| Brûleur allumé | 7398 – 7432 | 7385 – 7417 |
| Seuil (milieu des réglages ON / OFF) | 3828 | 4240 |

Marge très large des deux côtés : capteur bien positionné. La décision ON/OFF utilise le rouge et l'**intensité brute**
(canal « clair » du TCS34725), pas le « Lux » calculé, qui déborde (≈ 63 570) quand la DEL est allumée.

## 4. Défauts trouvés et corrections

| Défaut | Effet | Correction |
|---|---|---|
| Keep-alive : le CYD renvoyait la **commande** (`pulseBufferON/OFF`) au lieu de la trame de **maintien** | brûleur arrêté au bout de 1 à 9 min | `pulseBufferACK_ON/ACK_OFF` ; tables par défaut **7, 15, 56, 86, 626 s** (ON et OFF) |
| Au démarrage, le CYD renvoyait `boiler_state`, que la **mesure** de la DEL modifie | une mesure pouvait devenir une commande à la chaudière | nouvelle clé NVS **`chaud_cmd`** (dernière commande, jamais modifiée par la mesure) rejouée au démarrage ; rien si elle est absente |
| L'état réel était réappliqué à **chaque redessin de l'écran** avec l'ancienne mesure | `ONR` puis `OFFR` parasite après chaque commande, réécriture de `boiler_state` | état réel appliqué à la réception d'une trame `Del`/`DelR` (callback CoulSdb, `main_ThSdb.cpp`) ; valeur > 1 (couleur 433) ignorée |
| Mesure prise avant que le brûleur ait suivi la commande | `OFFR` 1 s après un ON : le bouton HA (toggle) envoyait ensuite l'inverse | **délai de grâce de 15 s** après une commande : une mesure contraire est ignorée |
| Fréquence figée dans `global.h` (`FREQUENCE`), champ de la page Web sans effet | réglage impossible sans recompiler | émission sur **`chaud_freq`** (page Web, toute précision), défaut **434,092 MHz**, garde-fou 433 – 435 MHz |
| DEL de ThSdb clignotante | — | pas un défaut : deep sleep actif (sorties coupées pendant le sommeil) ; deep sleep désactivé pour l'hiver |
| DEL de ThSdb : réglage « DEL OFF (0 ou 1) » ignoré | DEL inversée si câblée vers le +3,3 V | niveau calculé utilisé (Capteur 3.3) |

## 5. Réglages en service

| Où | Réglage |
|---|---|
| Page du CYD principal (192.168.1.94) | keep-alive **activé** ; tables ON et OFF **7, 15, 56, 86, 626** ; fréquence **434.092** |
| ThSdb (192.168.1.175) | seuils rouge 7595 / 61, intensité 8251 / 229 ; mesure toutes les 2 s ; deep sleep **désactivé** ; « Mesures brutes » à décocher après les essais |
| AD200 | **sans piles** : il émet toutes les 625,5 s même « en sommeil » et contredirait le CYD |

## 6. Limites et pistes

- **Deux émetteurs** : si l'AD200 est remis en service, le récepteur suivra la dernière trame des deux. Ne garder qu'un émetteur.
- **Bouton HA en toggle** : l'état affiché suit la DEL. Si la chaudière change d'état sans commande du CYD (AD200,
  panne), un appui envoie l'inverse de ce qui est voulu. Piste : boutons ON et OFF séparés.
- **La DEL montre le brûleur, pas la demande** : avec l'AD200 en chronoproportionnel près de la consigne, ou si la
  chaudière régule, la DEL peut s'éteindre pendant une demande de chauffe.
- **Dernière impulsion des trames de maintien** : longue au CYD, courte à l'AD200 ; sans effet constaté.
- **Ancien code du nœud chauffage SdB** (`CNoeudChauffageIRSound`) : utilisait les mêmes clés NVS que la chaudière
  (`chaud_*`, `boiler_state`) ; il est en commentaire, à corriger s'il est réactivé.

## Annexe — Commandes rtl_433

```
cd D:\Outils\rtl_433
rtl_433.exe -f 434.0M -s 1024k -A -M time:iso:usec -M level -R 0 ^
  -X "n=AD200,m=OOK_PWM,s=210,l=450,r=1500,g=700" -F kv -F json:captures\essai.json
```

`-A` : analyse des impulsions de chaque trame ; `-X` : décodeur générique (impulsion courte 210 µs, longue 450 µs) ;
`-F json` : une ligne par trame, avec l'heure, les données et la fréquence. Fermer URH et CubicSDR avant : le dongle
ne sert qu'à un programme à la fois.
