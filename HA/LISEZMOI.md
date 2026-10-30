# Configuration Home Assistant

Copie de référence du dossier `/config` de Home Assistant (https://ha.jbil.org), pour suivre ses versions avec le code des équipements.

Dernier export : 27/10/2026.

## Contenu

| Fichier | Rôle |
|---|---|
| `configuration.yaml` | Entités MQTT (switch, sensor), input_*, thermostat, notification e-mail, templates |
| `automations.yaml` | Automatisations (IP des équipements, CYD, cuve…) |
| `scripts.yaml` | Scripts |
| `packages/` | Paquets inclus par `homeassistant: packages: !include_dir_named packages` |
| `packages/noeuds_capteurs.yaml` | Pilotage à distance des nœuds capteurs LoRa (via Relais-Lora) et Wi-Fi (réglages, GET, STATUS, REBOOT, ANNULER, nettoyage du retenu, commande libre, éveil/sommeil) — **généré** |
| `packages/cyd.yaml` | CYD principal (`cyd_*`, topics `home/configuration/`) et CYD Aux (`cyd_aux_*`, topics `home/configaux/`, nom « Home Assistant Aux ») et CYD Aux2 (`cyd_aux2_*`, topics `home/configaux2/`, nom « Home Assistant Aux2 ») ; pour chacun : `sensor.cyd_etat` (réponse complète au STATUS, attribut `texte`), `binary_sensor.cyd_ecran` (écran allumé / éteint, Home-Assist 3.4), `binary_sensor.cyd_connexion` (dernier ALIVEWDOG plus récent que le délai d'alerte), `binary_sensor.cyd_redemarrage` (REBOOT en cours), `script.cyd_ecran_bascule` |
| `vue-cyd.yaml` | Vue « CYD » du tableau Système (connexion, IP, liens Page Web / Journal, redémarrage avec confirmation, bascule de l'écran, réglages, état en tableaux — chaudière et lumière cave : état mesuré par CoulSdb / JnCave), à coller dans l'interface |
| `vue-cyd-aux.yaml` | Vue « CYD Aux » : même contenu que `vue-cyd.yaml` (substitution `cyd_` → `cyd_aux_`), à coller dans l'interface |
| `vue-cyd-aux2.yaml` | Vue « CYD Aux 2 » : idem avec `cyd_aux2_`, à coller dans l'interface |
| `vue-noeuds-capteurs.yaml` | Vue « Nœuds capteurs » du tableau Système, à coller dans l'interface (pas un fichier de HA) — **générée** |
| `tableau-accueil-complet.yaml` | Tableau Accueil **entier** (6 vues, même contenu que `vue-accueil.yaml` pour la vue Accueil), pour l'**éditeur de configuration brute** (qui remplace tout le tableau) |
| `vue-accueil.yaml` | Vue « Accueil » du tableau Accueil (export `.storage` du 30/10/2026 + carte « Lumière cave » : état MESURÉ par ThCave, sensor.jncave_etat Jour = Allumée / Nuit = Éteinte, à côté de Chaudière et Cuve), à coller dans l'interface |
| `vue-actionneurs.yaml` | Vue « Actionneurs » du tableau Accueil (export `.storage` + carte du nœud chauffage SdB), à coller dans l'interface — **générée** |
| `genere_noeuds.py` | Générateur des trois fichiers ci-dessus à partir de la liste des nœuds (pas un fichier de HA) |
| `.storage/lovelace_dashboards` | Liste des tableaux de bord |
| `.storage/lovelace.dashboard_systeme` | Tableau « Système » (vues NAS, Diagnostic, CYD, Paramétrage) |
| `.storage/lovelace.dashboard_accueil` | Tableau « Accueil » (vues Accueil, Actionneurs, Chaudière, Graphs, Capteurs, Caméras) |

## Règles (le dépôt GitHub est public)

- Ne jamais copier `secrets.yaml` : les mots de passe y sont rangés et appelés par `!secret <nom>`
  (ex. `gmail_app_password` pour la notification e-mail).
- De `.storage`, ne copier que les fichiers `lovelace*` : les autres contiennent des identifiants.
- Avant un commit, vérifier qu'aucun mot de passe ou jeton n'est en clair.

## Refaire un export

1. Module File editor (ou Samba `\\<IP de HA>\config`) : télécharger `configuration.yaml`, `automations.yaml`,
   `scripts.yaml` et le dossier `packages`.
2. `.storage` est caché : dans File editor, retirer `.storage` de « Ignore Pattern » (onglet Configuration du module),
   enregistrer et redémarrer le module ; sinon, copier le tableau depuis ⋮ → Modifier → ⋮ → Éditeur de configuration brute.

## Appliquer une modification dans HA

- YAML : remplacer le fichier, puis Outils de développement → YAML → « Vérifier la configuration » avant de recharger ou redémarrer.
- Tableaux de bord : passer par l'interface (éditeur de carte ou éditeur de configuration brute), jamais en écrasant
  un fichier `.storage` pendant que HA tourne.

## Installer le pilotage des nœuds capteurs (paquet noeuds_capteurs)

Un **nœud** = une carte (un suffixe : Remise, Cave, ChRDC…) portant plusieurs capteurs unitaires (Th<Suffixe>,
Bat<Suffixe>, Flotteur<Suffixe>…). Identifiant du nœud dans HA : `th<suffixe>` (ex. `thchrdc`).
File d'attente des commandes : LoRa = Relais-Lora (publie `home/<conf>/attente`, traite `ANNULER`) ;
Wi-Fi = commande publiée en **retenu**, effacée par l'automatisation « Nœuds Wi-Fi - Effacer la commande retenue »
dès l'accusé du nœud ou sa connexion (« online »).

**Ajouter un nœud** : compléter la liste `NOEUDS` de `genere_noeuds.py` (mode, capteurs unitaires, entités de mesure),
lancer `python -I HA/genere_noeuds.py`, puis réinstaller les deux fichiers (étapes 1, 3 et 4). Un nœud LoRa doit
aussi figurer dans `FILE_CMD_CAPTEURS` (Relais-Lora/src/global.h).
Le générateur a besoin de **PyYAML** (pour `vue-actionneurs.yaml`) : `py -m pip install pyyaml` une fois ; sinon il
s'arrête après avoir écrit les deux premiers fichiers (`ModuleNotFoundError: No module named 'yaml'`).

**Détecteur jour / nuit** (classe `CJourNuit` du Capteur, trames `Jn<Suffixe> Jour …` et `Jn<Suffixe> Eclairement …`) :
dans `NOEUDS`, ajouter `jn=True`, l'unité `"Jn"` et les deux mesures `sensor.jn<suffixe>_etat` / `sensor.jn<suffixe>_eclairement`
(à créer dans `configuration.yaml`, sur le modèle de `JnCave Etat` / `JnCave Eclairement`). Effet : attributs `jn_*` de
`sensor.<id>_config` affichés **en lecture seule** dans la carte « Jour / nuit » du nœud (avec un rappel de la syntaxe),
modifiables par la commande libre (`SET jn_sbas 15 jn_shaut 25` …, détail dans l'aide de la vue). Seul ThCave l'a
aujourd'hui ; l'indicateur « Cave · Jour / Nuit » de la vue Actionneurs (`CARTE_JOUR_NUIT_CAVE`) lui est propre.

**Flotteur avec réveil et confirmation** (Capteur 3.1, classe `CTor`, préfixe `mfv_`) : dans `NOEUDS`, ajouter `fl=True`.
Effet : attributs `mfv_rev`, `mfv_revmin`, `mfv_ack`, `mfv_ackn` de `sensor.<id>_config` affichés **en lecture seule** dans
la carte « Flotteur » du nœud, modifiables par la commande libre (`SET mfv_revmin 300 mfv_ack 1` …). Seul ThRemise l'a.
La confirmation (« FlotteurRemise ACK ») est envoyée par le contrôleur principal (Home-Assist 3.3), plus par HA.

**Actionneur** (`type="actionneur"`, ex. nœud chauffage SdB) : pas de réglages de sommeil, hors de la vue « Nœuds
capteurs » ; sa carte est insérée dans `vue-actionneurs.yaml` (vue Actionneurs du tableau Accueil, reprise de
`.storage/lovelace.dashboard_accueil` : réexporter ce fichier avant de régénérer si la vue a été modifiée dans HA),
à coller : Accueil → ✏️ → vue Actionneurs → ⋮ → « Modifier en YAML »). Ses commandes ne sont pas retenues (toujours éveillé quand il est sous tension).

1. Dans `/config/packages/` : **supprimer `capteurs_lora.yaml`** (ancienne version, entités en double sinon) et copier
   `packages/noeuds_capteurs.yaml`.
2. `/config/automations.yaml` : l'automatisation « Cuve - SLEEP et ACK flotteur » a été **supprimée** le 30/10/2026
   (remplacée par le réveil par le flotteur et la confirmation par le contrôleur principal, Capteur 3.1 / Home-Assist 3.3).
3. Outils de développement → YAML → « Vérifier la configuration », puis redémarrer HA.
4. Tableau Système → ✏️ → vue « Capteurs LoRa » (ou « + » nouvelle vue) → ⋮ → « Modifier en YAML » → remplacer par
   `vue-noeuds-capteurs.yaml` → Enregistrer (nouvelle adresse : `/dashboard-systeme/noeuds-capteurs`).
5. Vérifier : `sensor.thremise_commande_en_attente` = « aucune » (message retenu du relais) ; bouton « Relire » →
   la commande apparaît en attente puis, au réveil suivant, « Config reçue » se met à jour et les réglages se remplissent.
   Nœud Wi-Fi : « Relire » → commande en attente = GET, puis « aucune » dès l'accusé, et « Config reçue » se met à jour.
