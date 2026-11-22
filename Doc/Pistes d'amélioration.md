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

### 1.2 Testaments « offline » au coucher (Wi-Fi)

- **Constat** (2026-11-15, test USB de ThChRDC, Capteur 3.8) : sur environ 3 cycles sur 10, le testament MQTT
  « offline » est publié au coucher (Echo 0 / Fin 0 dans Préc). La connexion MQTT est coupée avant le deep sleep sans
  déconnexion propre, et le broker publie le testament.
- **Effet** : faux « hors ligne » passagers côté HA ou CYD, jusqu'au réveil suivant.
- **Piste** : faire une déconnexion MQTT propre (`disconnect()`, puis attendre que la pile réseau ait vidé ses envois)
  avant de couper le Wi-Fi ; sinon publier explicitement l'état « endormi » avant de se déconnecter.

### 1.3 Valeur inchangée après MUTE OFF

- **Constat** (limite connue depuis Capteur 3.8, mémoire RTC) : en mode MUTE, les mesures sont déclarées envoyées. Après
  MUTE OFF, une valeur qui n'a pas changé n'est renvoyée qu'au prochain forçage.
- **Piste** : à la sortie du mode MUTE, ne pas mettre à jour la dernière valeur envoyée pendant le MUTE (ou la marquer
  invalide) pour que la première mesure suivante parte quel que soit l'écart.

## 2. CYD (contrôleur Home-Assist)

### 2.1 Réglages appliqués sans redémarrage (CYD et nœuds)

- **Constat** (22/11/2026) : sur les CYD comme sur les nœuds Capteur, « Sauvegarder » sur la page Web enregistre puis
  redémarre l'appareil. Une modification simple (code RF, case Actif, délai…) coupe tout : pilotage, mesures, connexion.
- **Piste** : appliquer à chaud ce qui le permet (valeurs relues en mémoire, objet concerné réinitialisé : code
  RCSwitch, cases Actif / Vérification, délais, seuils), et ne redémarrer que pour ce qui l'exige (Wi-Fi, broker MQTT,
  topics et abonnements, broches) ; la page indique alors ce qui demande un redémarrage.

## 3. Cartes et boîtiers

(Aucune piste pour l'instant ; voir 1.1 pour les cartes à piles.)

## 4. Nouveaux équipements

### 4.1 Détection de fumée et de monoxyde de carbone (CO) dans HA

Étude du 2026-11-19, **rien d'acheté ni de commencé**.

- **Principe retenu** : la sécurité reste assurée par des **détecteurs certifiés** (fumée **NF EN 14604**, obligatoire ;
  CO **EN 50291**), qui sonnent seuls, sans réseau. HA ne sert qu'aux notifications et automatisations (couper la
  chaudière ou la VMC, allumer les lumières…). Un détecteur 100 % DIY n'est pas une alarme de sécurité.
- **Solution recommandée : détecteurs certifiés Matter over Thread** (aucun développement) :
  - **Sensereo MSC-1** : fumée + CO, EN 14604 + EN 50291 annoncées, écran ppm, sur pile, Matter 1.4, testé avec HA →
    près de la chaudière ;
  - **Sensereo MS-1 Interlink** (fumée, EN 14604 TÜV, interconnexion sans fil) ou **Heiman HS1SA-M** (fumée,
    EN 14604 ; prendre la référence **européenne**, la S1-M nord-américaine est UL 217) → chambres, couloir ;
  - Heiman HS-720ES-M (CO) : certification EN 50291 **non confirmée**, à vérifier.
  - Avant achat : vérifier le **certificat** (organisme notifié : TÜV, VdS, AFNOR…) et la déclaration CE, pas
    seulement la fiche Amazon.
- **Prérequis HA** : un **Thread Border Router** → **déjà présent : OTBR sur ESP32** dans le système (info du
  2026-11-19) ; vérifier seulement le serveur Matter (add-on) + intégrations Matter et Thread dans HA, et que les
  détecteurs rejoignent bien ce réseau Thread. Le réseau comprend déjà des **prises Eve** (sur secteur = routeurs
  Thread) : elles servent de relais aux détecteurs sur pile, qui ne routent pas ; en ajouter une près d'un détecteur
  éloigné si besoin. Le type d'appareil Matter **Smoke CO Alarm** (0x0076) est
  reconnu par HA : entités fumée, CO, batterie, test, défaut, fin de vie (EndOfService), ppm si transmis.
- **Pose** : fumée au plafond, au centre de la pièce ; CO selon la notice (≈ 1 à 3 m de l'appareil à combustion).
  Durée de vie ≈ 10 ans (fumée), 7 à 10 ans (cellule CO).
- **Variantes DIY étudiées (si besoin un jour)** :
  - *ESP32 + MQTT (comme les nœuds actuels)* : détecteur de fumée **filaire 12 V à sortie relais** (type centrale
    d'alarme) lu par un **PC817** + module CO électrochimique **Winsen ZE07-CO** (UART) + buzzer local ; option
    **PMS5003** (particules) en pré-alerte. Alimentation secteur obligatoire. **À éviter** : MQ-2 / MQ-135 (pas des
    capteurs de fumée), MQ-7 (CO non fiable), chambre optique maison.
  - *ESP32 Matter over Thread* : **ESP32-C6** (secteur, routeur Thread) ou **ESP32-H2** (pile, Sleepy End Device,
    avec cellule **Figaro TGS5042** + ampli-op DIP) ; firmware **ESP-IDF + esp-matter** (endpoint `smoke_co_alarm`,
    PlatformIO inadapté) ; nettement plus lourd à développer. L'ESP32 classique / CYD n'a pas de radio Thread.
- **Rôle restant pour le DIY** : complément qualité de l'air (particules, CO₂), pas la sécurité.
- **Sources** : sensereo.com/msc-1, sensereo.com/product/ms-1, meshsmarthome.com (tests MSC-1 et MS-1),
  raspberry.tips (MSC-1 avec HA), localsmarthomeguide.com (Heiman UE / US), matteralpha.com (liste Smoke CO Alarm).
