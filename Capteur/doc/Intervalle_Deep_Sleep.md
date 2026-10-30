# Intervalle de deep sleep imposé par Home Assistant

Capteurs avec flotteur (ThRemise, ThNomade) — octobre 2026.

> **HISTORIQUE — 30/10/2026 : l'automatisation « Cuve - SLEEP et ACK flotteur » est SUPPRIMÉE.** Elle est remplacée par
> le réveil par le flotteur et la confirmation par le contrôleur principal (Capteur 3.1, Home-Assist 3.3 : voir
> `pcb/Capteur S3/CapteurS3.md` § 1). L'intervalle de deep sleep n'est plus modifié par Home Assistant ; ce qui suit ne
> sert plus qu'à comprendre les anciennes notes.

## 1. Symptôme

Sur la page Web du capteur, on modifie « Intervalle deep sleep » et on enregistre. Quelques minutes plus tard, la
valeur a changé toute seule. La case « Deep Sleep » peut aussi se recocher, même avec `DEBUG_NO_DEEP_SLEEP`.

Dans MQTTBox, on voit passer sur le topic d'état du capteur :

```
ThRemise COMMANDE reçue à 124137 ms : SLEEP SLEEPL (deep sleep : ACTIF)
```

## 2. Cause

Ce n'est pas un défaut : c'est l'automatisation Home Assistant **« Cuve - SLEEP et ACK flotteur »**
(`Doc/automations.yaml`).

1. Le capteur publie l'état du flotteur (`FLOTTEURREMISE …` sur `home/thermometre/state`).
2. L'automatisation répond sur `home/confthremise/command`, selon la météo :
   - pluie probable (> 80 %) ou annoncée : `SLEEP SLEEPS` (sommeil court, la cuve est surveillée souvent) ;
   - sinon : `SLEEP SLEEPL` (sommeil long).
3. Deux secondes après, elle envoie `FLOTTEURREMISE ACK` sur `home/thermometre/command` (accusé du flotteur).

À la réception de `SLEEPS` ou `SLEEPL`, le capteur :

- remplace « Intervalle deep sleep » par la durée **Small** ou **Large** de la page Web, et l'enregistre ;
- active le deep sleep.

Le message n'est pas conservé (`retain: false`) : il arrive en réponse au flotteur, donc environ 2 minutes après le
démarrage, pas dès la connexion.

En fonctionnement normal, les durées utiles sont donc **Small** et **Large**, pas « Intervalle deep sleep ».

## 3. Procédure pour les essais (intervalle fixe)

1. Home Assistant : Paramètres → Automatisations → « Cuve - SLEEP et ACK flotteur » → menu ⋮ → **Désactiver**.
2. Page Web du capteur : régler « Intervalle deep sleep », enregistrer. La valeur ne bouge plus.
3. Si le capteur est en LoRa : **décocher « ACK nécessaire »** sur le flotteur. Sans l'automatisation, l'ACK
   n'arrive plus et le capteur ne s'endort qu'au bout de sa durée maximale d'éveil. (En Wi-Fi seul, l'ACK n'empêche
   pas le sommeil.)

## 4. Après les essais

1. **Réactiver l'automatisation** dans Home Assistant (sinon plus de surveillance rapprochée de la cuve par temps
   de pluie, et plus d'ACK du flotteur).
2. Recocher « ACK nécessaire » si on l'avait décoché.
3. Régler les durées **Small** et **Large** sur la page Web : ce sont elles que l'automatisation applique.
