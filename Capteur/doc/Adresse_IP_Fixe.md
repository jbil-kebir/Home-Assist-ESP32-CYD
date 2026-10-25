# Adresse IP fixe et canal mémorisé (connexion Wi-Fi rapide)

Capteurs Wi-Fi sur piles (XIAO ESP32-C3, ESP32-S3) — octobre 2026.

## 1. But

Un capteur sur piles passe l'essentiel de son temps en deep sleep (environ 43 µA). C'est l'éveil qui vide les
piles : le Wi-Fi consomme de 20 à 90 mA. Raccourcir l'éveil allonge donc directement l'autonomie.

Mesures sur le capteur ThChRDC (octobre 2026, après les corrections du deep sleep) :

| Étape de l'éveil | Durée |
|---|---|
| Démarrage et connexion Wi-Fi | 3,4 s |
| Connexion MQTT, publications, écho, déconnexion | 1,3 s |
| **Total** | **4,7 s** |

La connexion Wi-Fi représente à elle seule près des trois quarts de l'éveil. Les deux options décrites ici
visent à la ramener vers 1 s.

## 2. Principe

Sans option, à chaque réveil, le capteur refait toute la connexion depuis le début :

1. **Recherche de la box** : il écoute les 13 canaux Wi-Fi un par un pour la retrouver.
2. **Demande d'adresse (DHCP)** : la box lui attribue une adresse IP, en plusieurs allers-retours.

Les deux options suppriment ces étapes. Elles sont **indépendantes** : on peut activer l'une, l'autre ou les deux.

| Option | Ce qu'elle supprime | Où l'information est gardée |
|---|---|---|
| **IP fixe** | La demande d'adresse (DHCP) | Mémoire permanente (NVS), réglée sur la page Web |
| **Mémoriser le canal** | La recherche de la box sur tous les canaux | Mémoire RTC (conservée pendant le deep sleep) |

### 2.1 IP fixe

Le capteur utilise toujours la même adresse, sans la demander à la box. Il faut lui indiquer aussi
la passerelle (la box), le masque du réseau et le serveur DNS.

**Précaution** : la box ne doit pas pouvoir donner cette adresse à un autre appareil, sinon deux appareils
auraient la même adresse et aucun des deux ne fonctionnerait correctement. On **réserve donc l'adresse dans
la box** (voir § 3).

### 2.2 Mémoriser le canal

Après chaque connexion réussie, le capteur note en mémoire RTC le **canal** de la box et son **BSSID**
(l'identifiant unique de la borne Wi-Fi). Au réveil suivant, il se connecte directement sur ce canal, à
cette borne, sans rien chercher.

- **Si la connexion rapide échoue** (la box a changé de canal, après un redémarrage par exemple), le capteur
  abandonne au bout de **3 s** et refait une connexion normale. Il ne perd aucune mesure, seulement un peu de temps.
- **À la mise sous tension** (piles changées, téléversement), la mémoire RTC est vide : le premier éveil se
  fait en connexion normale, puis les suivants sont rapides.
- Le canal n'est réutilisé que pour le **même réseau** (même SSID).

## 3. Réserver l'adresse dans la Freebox Pop

À faire une seule fois par capteur, depuis un PC connecté au réseau de la maison :

1. Ouvrir **http://mafreebox.freebox.fr** et se connecter avec le mot de passe de la Freebox.
2. Ouvrir **Paramètres de la Freebox** (passer en **Mode avancé** si besoin).
3. Cliquer sur **DHCP**.
4. Onglet **Baux dynamiques** : repérer la ligne de l'adresse actuelle du capteur (par exemple 192.168.1.128)
   et noter son **adresse MAC**.
5. Onglet **Baux statiques** : **Ajouter un bail DHCP statique**, choisir l'adresse MAC du capteur, saisir
   l'adresse IP et un commentaire (par exemple « ThChRDC »), puis valider.

Les intitulés peuvent varier légèrement selon la version de Freebox OS.

Réglages réseau de la Freebox Pop, à reporter sur la page Web du capteur :

| Champ | Valeur |
|---|---|
| Passerelle | 192.168.1.254 |
| Masque | 255.255.255.0 |
| DNS | 192.168.1.254 |

## 4. Activer les options sur la page Web du capteur

En fonctionnement normal, le capteur n'est éveillé que quelques secondes : sa page Web est pratiquement
inaccessible. Il faut d'abord le **garder éveillé**.

1. **Désactiver le deep sleep** : publier `SLEEP DISABLE` sur le topic de configuration du capteur, en
   **retained** (le capteur le reçoit à son prochain réveil). Pour ThChRDC : `home/confthchrdc/command`.
2. Ouvrir la page Web du capteur : **http://192.168.1.128** (adresse du capteur).
3. Dans le cadre **Wifi** du réseau utilisé :
   - cocher **IP fixe** et remplir **Adresse IP**, **Passerelle**, **Masque** et **DNS** (§ 3) ;
   - cocher **Mémoriser le canal**.
4. **Enregistrer**. Le capteur redémarre avec les nouveaux réglages.
5. **Réactiver le deep sleep** : publier `SLEEP ENABLE` sur le même topic (en retained, pour remplacer
   le `SLEEP DISABLE`).

Champs ajoutés au cadre Wifi :

| Champ | Rôle | Valeur par défaut |
|---|---|---|
| IP fixe | Active l'adresse fixe (sinon DHCP) | Décoché |
| Mémoriser le canal | Active la connexion sur le canal mémorisé | Décoché |
| Adresse IP | Adresse du capteur, réservée dans la box | Vide (l'adresse actuelle est affichée en grisé) |
| Passerelle | Adresse de la box | 192.168.1.254 |
| Masque | Masque du réseau | 255.255.255.0 |
| DNS | Serveur de noms (si vide ou invalide : la passerelle) | 192.168.1.254 |

**Sécurité** : si l'adresse IP, la passerelle ou le masque sont mal écrits, le capteur l'indique dans le
journal (« IP fixe invalide : DHCP ») et revient au DHCP. Il reste donc joignable.

Pour revenir au fonctionnement d'origine : décocher les deux cases et enregistrer.

## 5. Vérifier que ça marche

Le message `REVEIL` publié à chaque réveil contient le diagnostic du cycle précédent (« Préc : »).
Le premier nombre est l'instant, en millisecondes depuis le réveil, où le Wi-Fi est connecté :

```
Préc : WiFi 3394 NTP 3394 MQTT 3466 (1 essai) Pub 3598 Echo 4947 Fin 4974 Dodo 5050
Préc : WiFi 1100 (canal mémorisé) NTP 1100 MQTT 1170 (1 essai) Pub 1300 ...
Préc : WiFi 6500 (canal mémorisé : échec) NTP 6500 ...
```

- **Sans mention** : connexion normale.
- **(canal mémorisé)** : connexion rapide réussie.
- **(canal mémorisé : échec)** : la connexion rapide a échoué et le capteur a fait une connexion normale.
  Si cela arrive souvent, la box change de canal (canal automatique) : on peut fixer le canal Wi-Fi dans la
  box, ou décocher **Mémoriser le canal**.

La ligne « Durée éveil » du message `DEEPSLEEP ON` donne la durée totale de l'éveil.

## 6. Dépannage

| Symptôme | Cause probable | Solution |
|---|---|---|
| Le capteur ne publie plus rien | Adresse IP déjà utilisée, ou mauvaise passerelle | Vérifier la réservation dans la box (§ 3) et la passerelle |
| « canal mémorisé : échec » à chaque cycle | La box change de canal, ou le capteur capte plusieurs bornes | Fixer le canal dans la box, ou décocher Mémoriser le canal |
| Plus de résolution des noms (serveur NTP) | DNS incorrect | Mettre l'adresse de la box dans DNS |
| Page Web injoignable | Le capteur dort | `SLEEP DISABLE` en retained (§ 4) |

## 7. Fichiers concernés (programme Capteur)

| Fichier | Contenu |
|---|---|
| `src/MyWifi.h` | Champs IP fixe et canal mémorisé, délais `WIFI_TIMEOUT_MS` (15 s) et `WIFI_TIMEOUT_RAPIDE_MS` (3 s) |
| `src/MyWifi.cpp` | Connexion (`begin()`), mémoire RTC du canal, page Web, enregistrement NVS |
| `src/Diagnostic.h/.cpp` | Mention « canal mémorisé » dans le diagnostic du message REVEIL |
