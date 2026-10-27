# Pilotage_Maison

Toujours répondre en français. Messages de commit en français.

## Dépôt git et workflow de commit

`D:\Developpement\Pilotage_Maison` est la **copie de développement locale, sans `.git`** : `git status` y échoue ("not a git repository").

Le dépôt git de référence est sur le NAS :
`\\newnas\Kebir\Info-Developpement\GitHub\Pilotage_Maison`
- remote `origin` = https://github.com/jbil-kebir/Home-Assist-ESP32-CYD.git
- branche `main`

Correspondance des dossiers D: → NAS : `Capteur`, `CydMonitor`, `Home-Assist`, `Relais-Lora`, `Linky`, `Doc`, `Firmwares`, `images`, `HA` (même nom) ; `Home-Assist-App 1.2` → `Home-Assist-App`.
Ne pas copier : `secret.h` (identifiants Wi-Fi / MQTT réels, exclu par `.gitignore` ; seul `secret.exemple.h` est commité), `Archives`, `Analyse`, `Programmes unitaires`, les `.zip`, les raccourcis `.lnk`, les sauvegardes datées.

`HA` = copie de la configuration Home Assistant (`/config`), voir `HA/LISEZMOI.md`. Dépôt public : jamais `secrets.yaml`, et de `.storage` seulement les fichiers `lovelace*` ; vérifier qu'aucun mot de passe ou jeton n'est en clair avant de commiter.

### Procédure « commit et pousse »

1. **Comparer tout le dossier du projet**, pas seulement les fichiers modifiés dans la session (les deux copies peuvent avoir divergé lors de sessions précédentes) :
   ```bash
   diff -rq --exclude=.pio --exclude=.vscode --exclude='~$*' --exclude='*.bak.docx' \
     "D:/Developpement/Pilotage_Maison/Capteur" \
     "//newnas/Kebir/Info-Developpement/GitHub/Pilotage_Maison/Capteur"
   ```
   Si l'écart est plus large que prévu, confirmer le périmètre avec l'utilisateur.
2. **Copier D: → NAS** les fichiers concernés (cp, ou robocopy pour un dossier entier), en excluant `.pio`, `.vscode`, `~$*.docx`, `*.bak.docx`.
3. **Commiter et pousser depuis le NAS** :
   ```bash
   cd "//newnas/Kebir/Info-Developpement/GitHub/Pilotage_Maison"
   git add <fichiers> && git commit && git push origin main
   ```
   Les chemins UNC fonctionnent avec git/bash, mais sont lents (status/diff : plusieurs secondes).
4. Avant de commiter un capteur, vérifier les réglages DEBUG / PRODUCTION de `platformio.ini` et `global.h` et signaler s'ils sont en mode debug (consommation batterie).
