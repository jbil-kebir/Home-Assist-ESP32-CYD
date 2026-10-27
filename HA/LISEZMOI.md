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
| `.storage/lovelace_dashboards` | Liste des tableaux de bord |
| `.storage/lovelace.dashboard_systeme` | Tableau « Système » (vues NAS, Diagnostic, CYD, Paramétrage) |

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
