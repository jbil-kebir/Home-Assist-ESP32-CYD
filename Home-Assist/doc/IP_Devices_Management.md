# Gestion des Adresses IP des Appareils - Documentation Complète

## Vue d'ensemble

Le système **Home-Assist 2.7** collecte dynamiquement les adresses IP de tous les appareils connectés via MQTT et les affiche dans l'**écran série 5**. Ces informations sont extraites des messages MQTT, stockées en mémoire RAM (vecteurs), et remontées aux CYD auxiliaires.

---

## 1. Structure de Données

### CIPModule (Classe Conteneur)

**Fichier:** `src/MyIPModule.h` / `src/MyIPModule.cpp`

Chaque appareil connu est stocké dans un `CIPModule` contenant :
- **msNom** — Nom de l'appareil (ex: "ThSdb", "ThCave", "CYD HA")
- **msIP** — Adresse IP au format "xxx.xxx.xxx.xxx"

### Vecteurs de Stockage

**Fichier:** `src/main.cpp`

Deux vecteurs partagés entre Config, Écran et Zone d'affichage :
```cpp
std::vector<CIPModule> mvsControleurs;  // Contrôleurs/maîtres
std::vector<CIPModule> mvsEsclaves;     // Capteurs/actionneurs
```

**Partage des vecteurs:**
- Config lors du setup (ligne 168)
- Écran lors du setup (ligne 180)
- CZoneAffichageIP lors de l'initialisation

---

## 2. Extraction des Adresses IP depuis MQTT

### Format des Messages MQTT

**Pattern reçu:** `[NOM] [DONNEES...] [DATE] [HEURE] [IP] [FORCE?]`

**Exemple:**
```
THSDB 23.5 45 31/03/2026 13:47:39 192.168.1.67 FORCE
COULSDB DEL 31/03/2026 13:47:39 1 FORCE 192.168.1.67
DEVICE ThSdb 31/03/2026 13:47:39 192.168.1.45
```

### Classe ParsedMqttMessage

**Fichier:** `src/ParsedMqttMessage.cpp`

**Validation d'une IP (lignes 8-19):**
```cpp
bool ParsedMqttMessage::isIp(const String& s) {
    // Vérifie: 3 points, tous chiffres, min 7 chars
    // Exemple: "192.168.1.67" ✓
}
```

**Parsing du message (lignes 31-118):**

1. Divise le payload par espaces (tokenisation)
2. Premier token = nom de l'émetteur (`msExpediteur`)
3. Recherche les composants:
   - **msIp** — Première chaîne validée comme IP
   - **msDate** — Première date trouvée
   - **msTime** — Première heure trouvée
   - **mbForce** — Drapeau "FORCE" présent?
   - **mvsMesure** — Vecteur de mesures

**Retour:** Objet `ParsedMqttMessage` avec tous les champs peuplés

---

## 3. Flux de Traitement MQTT

### Point d'Entrée: MyMqtt::callback()

**Fichier:** `src/MyMqtt.cpp` lignes 342-369

```cpp
// Détection d'un nouveau CYD/appareil sur le réseau
else if (message.startsWith("DEVICE ")) {
    #ifdef __LOCAL_MODE__
    // En MODE LOCAL: on est le maître

    // Étape 1: Envoyer l'état de TOUS les appareils aux nouveaux venus
    mConfig.chaudiere->remonteStatusParMqtt();
    mConfig.projecteur->remonteStatusParMqtt();
    // ... tous les appareils ...

    // Étape 2: Traiter le message et enregistrer le nouvel appareil
    mConfig.remonteCYDInfoToEcran(messageOrg);
    #endif
}
```

### Étape 2: MyConfig::remonteCYDInfoToEcran()

**Fichier:** `src/MyConfig.cpp` lignes 146-166

```cpp
void CConfig::remonteCYDInfoToEcran(const String& payload) {
    // Étape 1: Parser le message
    ParsedMqttMessage pmsg;
    pmsg.parse(payload);

    // Étape 2: Appeler le callback avec nom et IP extraits
    if (onEquipement != nullptr) {
        onEquipement(pmsg.msExpediteur, pmsg.msIp);
    }
}
```

### Étape 3: Callback d'Enregistrement

**Fichier:** `src/main.cpp` lignes 90-101

**Fonction:** `ajouterControleur(const String& nom, const String& ip)`

```cpp
void ajouterControleur(const String& nom, const String& ip) {
    // Sécurité
    if (nom.isEmpty() || ip.isEmpty()) return;

    // Étape 1: Vérifier si l'appareil existe déjà
    for (const auto& ctrl : mvsControleurs) {
        if (ctrl.getNom() == nom) return;  // Pas d'ajout dupliqué
    }

    // Étape 2: Ajouter le nouvel appareil
    mvsControleurs.emplace_back(nom, ip);

    // Étape 3: Redessiner l'interface
    ecran.drawMainInterface();
}
```

**Résultat:** Le nouvel appareil est enregistré dans le vecteur partagé

---

## 4. Initialisation du Chaînage

**Fichier:** `src/main.cpp` fonction `setup()`

```cpp
// 1. Initialiser Config avec les vecteurs
config.setup("cfg_", &mvsControleurs, &mvsEsclaves);  // Ligne 168

// 2. Enregistrer le callback pour l'ajout d'appareils
config.setonEquipementCallback(ajouterControleur);    // Ligne 169-171

// 3. Initialiser Écran avec les mêmes vecteurs
ecran.setup("ecr", &mvsControleurs, &mvsEsclaves);    // Ligne 180

// 4. Ajouter le CYD lui-même à la liste
ajouterControleur(NOM_EQUIPEMENT, WiFi.localIP());    // Ligne 224

// 5. Debug: Afficher les appareils connus
printControleurs();   // Ligne 226
printEsclaves();      // Ligne 227
```

---

## 5. Affichage Écran - Série 5

### Activation de la Série 5

**Fichier:** `src/MyEcran.cpp` lignes 437-439

```cpp
else if (mucSerieAffichageEnCours == 5) {
    updateControleursEtEsclaves();
}
```

### Fonction d'Affichage

**Fichier:** `src/MyEcran.cpp` lignes 695-697

```cpp
void CEcran::updateControleursEtEsclaves() {
    mZoneAffichageIP.drawEquipements();
}
```

### Rendu Graphique

**Fichier:** `src/MyZoneAffichageIP.cpp` lignes 56-115

**Fonction:** `drawEquipements()`

```cpp
void CZoneAffichageIP::drawEquipements() {
    // Titre
    drawString("Controleurs et clients connus");

    // Boucle sur tous les appareils connus
    for (const auto& ctrl : *mvsControleurs) {
        // Afficher le nom (tronqué à 60 chars) à X
        drawString(ctrl.getNom(), X, Y);

        // Afficher l'IP à X+190
        drawString(ctrl.getIP(), X + 190, Y);

        // Ligne suivante
        Y += 15;
    }
}
```

**Résultat à l'écran:**
```
Controleurs et clients connus
ThSdb                                           192.168.1.45
ThCave                                          192.168.1.52
THCHRDC                                         192.168.1.61
CYD Master                                      192.168.1.67
```

---

## 6. Mode LOCAL_MODE vs REMOTE_MODE

### LOCAL_MODE (Maître - Transmet les États)

**Compilation:** `#define __LOCAL_MODE__` en dur

**Comportement:**
- A accès au transmetteur RF (CC1101)
- Reçoit les messages MQTT de TOUS les appareils
- Parse les messages et extrait les IPs
- Enregistre les appareils dans les vecteurs
- Diffuse les états à tous les CYD auxiliaires
- Affiche la série 5 avec toutes les IPs connues

### REMOTE_MODE (Esclave CYD AUX - Reçoit les États)

**Compilation:** `#define __LOCAL_MODE__` commenté

**Comportement:**
- CYD auxiliaire sur le réseau
- N'a PAS accès au RF
- Reçoit les états des appareils depuis le maître
- Affiche les appareils connus reçus du maître
- Peut aussi afficher la série 5 (liste partagée)

---

## 7. Transmission aux CYD Auxiliaires

### Quand un Nouvel Appareil Rejoint le Réseau

**Fichier:** `src/MyMqtt.cpp` lignes 342-369

Lorsqu'un CYD/appareil envoie "DEVICE ...", le maître en LOCAL_MODE:

```
1. Reçoit le message "DEVICE ThSdb 31/03/2026 13:47:39 192.168.1.45"
2. Enregistre ThSdb → 192.168.1.45
3. Publie sur MQTT tous les états actuels:
   - home/chaudiere/command → ONR/OFFR
   - home/thermometre/state → THSDB 23.5 ...
   - home/thermometre/state → THCAVE 18.2 ...
   - ... TOUS les appareils ...
```

### Remontée de l'État Écran

**Fichier:** `src/MyEcran.cpp` lignes 463-470

```cpp
int CEcran::remonteEcranSerieParMqtt() {
    String s = "ECRAN SERIE " + String(mucSerieAffichageEnCours);
    if (mConfig.onMqttPublish != nullptr) {
        mConfig.onMqttPublish(
            mConfig.topic_config_state.c_str(),
            s.c_str()
        );
        // Publie: "ECRAN SERIE 5"
        // Topic: home/configuration/state (ou home/configaux/state)
    }
}
```

---

## 8. Stockage et Persistance

### RAM (Runtime)

**Vecteurs:** `mvsControleurs`, `mvsEsclaves` stored in RAM

- **Avantage:** Pas de latence d'accès, simple à manipuler
- **Inconvénient:** Perdu au redémarrage

### NVS (EEPROM) - PAS utilisé

Les adresses IP ne sont **PAS persistées en EEPROM** car:
1. Les appareils peuvent changer d'IP (DHCP)
2. Reconstruction automatique à chaque démarrage via MQTT
3. Les capteurs re-publient au redémarrage du maître

### Persistance Effective

Les informations survivent au redémarrage du **maître** grâce au système MQTT:
- Maître redémarre → subscribe aux topics
- CYD auxiliaires envoient "DEVICE" messages
- Maître reconstruit les vecteurs dynamiquement

---

## 9. Schéma Complet du Flux de Données

```
┌─────────────────────────────────────────────────────────────────┐
│                       MQTT Broker                               │
│  home/thermometre/state  (publishing THSDB, THCAVE, etc...)    │
│  home/configuration/command  (reçoit DEVICE messages)           │
└──────────────────────┬────────────────────────────────────────┘
                       │
        ┌──────────────┴──────────────┐
        │                             │
        ▼                             ▼
    ThSdb Device               CYD Master (__LOCAL_MODE__)
    (sends MQTT)               (receives & processes)
                                      │
                                      │
                    ┌─────────────────┼─────────────────┐
                    │                 │                 │
                    ▼                 ▼                 ▼
            MyMqtt::callback()   ParsedMqttMessage  remonteCYDInfoToEcran()
            (détecte "DEVICE")   (extrait IP+nom)   (callback registration)
                    │                                    │
                    │                                    ▼
                    │                          ajouterControleur()
                    │                          (ajoute au vecteur)
                    │                                    │
                    ▼                                    ▼
            mvsControleurs vector (RAM)
                    │
        ┌───────────┴──────────────┐
        │                          │
        ▼                          ▼
    MyEcran                    CYD AUX
    (affiche série 5)   (reçoit states via MQTT)
                        │
                        ▼
                    MyZoneAffichageIP
                    (drawEquipements)
                        │
                        ▼
                    ÉCRAN: "Controleurs et clients
                           ThSdb  192.168.1.45
                           ThCave 192.168.1.52"
```

---

## 10. Fichiers Clés et Leurs Rôles

| Fichier | Fonction Clé | Lignes |
|---------|-------------|--------|
| `MyIPModule.h/cpp` | Classe CIPModule (conteneur nom+IP) | - |
| `ParsedMqttMessage.cpp` | Extraction IP du payload MQTT | 8-19 (isIp), 31-118 (parse) |
| `MyMqtt.cpp` | Réception messages MQTT + dispatch | 342-369 (callback) |
| `MyConfig.cpp` | Callback remontée info CYD | 146-166 (remonteCYDInfoToEcran) |
| `main.cpp` | Vecteurs partagés + ajouterControleur | 90-101, 168-171, 224-227 |
| `MyEcran.cpp` | Sélection série 5 + remontée écran | 437-439, 695-697, 463-470 |
| `MyZoneAffichageIP.cpp` | Rendu graphique IP | 56-115 (drawEquipements) |

---

## 11. Cas d'Usage: Un Nouveau Capteur se Connecte

```
ÉTAPE 1 — Capteur ThSdb envoie sur MQTT:
   Topic: home/configuration/command
   Payload: "DEVICE ThSdb 31/03/2026 13:47:39 192.168.1.45"

ÉTAPE 2 — CYD Master reçoit (LOCAL_MODE):
   MyMqtt::callback() détecte startsWith("DEVICE ")

ÉTAPE 3 — Parsing et Enregistrement:
   ParsedMqttMessage extrait:
   - msExpediteur = "ThSdb"
   - msIp = "192.168.1.45"

ÉTAPE 4 — Callback enregistrement:
   ajouterControleur("ThSdb", "192.168.1.45")
   → Ajoute à mvsControleurs

ÉTAPE 5 — Diffusion à tous:
   Publie tous les états actuels sur MQTT

ÉTAPE 6 — Affichage:
   Écran série 5 affiche:
   "ThSdb                                      192.168.1.45"

ÉTAPE 7 — CYD AUX reçoit via MQTT:
   Reçoit les states, reconstruit sa liste locale
   Affiche aussi la série 5
```

---

## Conclusion

Le système fonctionne en **4 phases clés:**

1. **Extraction** — ParsedMqttMessage valide et extrait les IPs
2. **Stockage** — Vecteurs RAM partagés entre modules
3. **Affichage** — Écran série 5 itère les vecteurs
4. **Distribution** — MQTT remonte les états aux CYD auxiliaires

Les IPs ne sont jamais persistées en EEPROM, mais le système est **auto-guérisseur** grâce à MQTT — tout redémarrage reconstruit automatiquement la liste des appareils connus.
