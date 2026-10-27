# -*- coding: utf-8 -*-
# Génère HA/packages/noeuds_capteurs.yaml et HA/vue-noeuds-capteurs.yaml à partir de la liste des nœuds.
# Lancer depuis n'importe où : python -I HA/genere_noeuds.py (écrit dans le dossier du script)
import os

HA = os.path.dirname(os.path.abspath(__file__))  # dossier HA du projet

# id = th<suffixe> (topics home/conf<id>/...) ; nom = nom de l'équipement ; mode = lora / wifi
# unites = préfixes des capteurs unitaires du nœud (trames « <Prefixe><Suffixe> ... » sur home/thermometre/state)
# mesures = (entité, nom affiché) du bloc Mesures (entités existantes de configuration.yaml)
NOEUDS = [
    dict(id="thremise", nom="ThRemise", suffixe="Remise", mode="lora", unites=["Th", "Bat", "Flotteur"],
         mesures=[("sensor.thremise_temperature", "Température"), ("sensor.thremise_humidite", "Humidité"),
                  ("sensor.batremise_tension", "Batterie"), ("sensor.batremise_etat", "État batterie"),
                  ("sensor.flotteur_remise", "Flotteur")]),
    # ThNomade : pas encore d'entités de mesure dans configuration.yaml
    dict(id="thnomade", nom="ThNomade", suffixe="Nomade", mode="lora", unites=["Th", "Bat", "Flotteur"], mesures=[]),
    dict(id="thcave", nom="ThCave", suffixe="Cave", mode="lora", unites=["Th", "Bat"],
         mesures=[("sensor.thcave_temperature", "Température"), ("sensor.thcave_humidite", "Humidité"),
                  ("sensor.batcave_tension", "Batterie"), ("sensor.batcave_etat", "État batterie")]),
    dict(id="thchrdc", nom="ThChRDC", suffixe="ChRDC", mode="wifi", unites=["Th", "Bat"],
         mesures=[("sensor.thchrdc_temperature", "Température"), ("sensor.thchrdc_humidite", "Humidité"),
                  ("sensor.batchrdc_tension", "Batterie"), ("sensor.batchrdc_etat", "État batterie")]),
]
LORA = [n["id"] for n in NOEUDS if n["mode"] == "lora"]
WIFI = [n["id"] for n in NOEUDS if n["mode"] == "wifi"]
PARAMS = [("sleepi", "Intervalle deep sleep", 86400, "mdi:timer-outline"),
          ("sleeps", "Sommeil court (SLEEPS)", 86400, "mdi:timer-outline"),
          ("sleepm", "Sommeil moyen (SLEEPM)", 86400, "mdi:timer-outline"),
          ("sleepl", "Sommeil long (SLEEPL)", 86400, "mdi:timer-outline"),
          ("wakei", "Éveil maximum", 3600, "mdi:timer-sand")]


def liste_yaml(l):
    return "[" + ", ".join("'%s'" % x for x in l) + "]"


# ============================================================ PAQUET
def paquet():
    o = []
    w = o.append
    w("""# ============================================================
# PACKAGE : NŒUDS CAPTEURS - pilotage à distance des nœuds (LoRa et Wi-Fi)
# Généré par genere_noeuds.py : modifier la liste NOEUDS du générateur plutôt que ce fichier.
#
# Un nœud = une carte (un suffixe : Remise, Cave, ChRDC...) portant plusieurs capteurs unitaires
# (Th<Suffixe>, Bat<Suffixe>, Flotteur<Suffixe>...). Identifiant du nœud : th<suffixe> (ex. thchrdc).
#
# Topics : home/conf<id>/command  commandes
#          home/conf<id>/state    réponses du nœud (CONFIG, ETAT, accusés « COMMANDE reçue »)
#          home/conf<id>/attente  LoRa : commande en attente dans le relais (retenu, publié par le relais)
# File d'attente : LoRa = le relais (envoi au prochain réveil) ;
#                  Wi-Fi = commande publiée en RETENU, effacée par HA à l'accusé ou à la connexion (« online »).
# Entités : sensor.<id>_config (attributs sleepA, sleepI, sleepS, sleepM, sleepL, wakeI, erreur)
#           sensor.<id>_rapport (réponse STATUS), sensor.<id>_commande_en_attente
#           sensor.<id>_derniere_trame, binary_sensor.<id>_eveille
#           input_boolean.<id>_sleepa, input_number.<id>_sleepi / _sleeps / _sleepm / _sleepl / _wakei
#           input_text.<id>_commande (commande libre)
# Les commandes sont décrites dans Doc/mqtt.docx § 3.2 (et l'aide « ? » de la page du relais).
# ============================================================

# Pas de bloc « mqtt: » ici : il est déjà un dictionnaire dans configuration.yaml (fusion de paquet refusée).
# Les capteurs MQTT sont donc des templates à déclencheur MQTT (comme packages/chauffage.yaml).
template:""")
    for n in NOEUDS:
        i, nom = n["id"], n["nom"]
        if n["mode"] == "lora":
            w(f"""  # ---------- {nom} : commande en attente dans le relais (« STATUS (envoi 1/10) » ou « aucune », retenu) ----------
  - trigger:
      - trigger: mqtt
        topic: "home/conf{i}/attente"
    sensor:
      - name: "{nom} Commande en attente"
        unique_id: {i}_commande_attente
        icon: mdi:timer-sand
        state: "{{{{ trigger.payload[:255] }}}}"
""")
        else:
            w(f"""  # ---------- {nom} : commande en attente = message retenu sur le topic command (vide = « aucune ») ----------
  - trigger:
      - trigger: mqtt
        topic: "home/conf{i}/command"
    sensor:
      - name: "{nom} Commande en attente"
        unique_id: {i}_commande_attente
        icon: mdi:timer-sand
        state: "{{{{ trigger.payload[:255] if trigger.payload | trim != '' else 'aucune' }}}}"
""")
        w(f"""  # ---------- {nom} : réponse CONFIG (« {nom} CONFIG sleepA:1 sleepI:30 ... [ERREUR cle:val] <IP> ») ----------
  - trigger:
      - trigger: mqtt
        topic: "home/conf{i}/state"
        value_template: "{{{{ (value.split(' ') + [''])[1] | upper }}}}"
        payload: "CONFIG"
    sensor:
      - name: "{nom} Config"
        unique_id: {i}_config
        device_class: timestamp
        icon: mdi:cog
        state: "{{{{ now().isoformat() }}}}"
        attributes:""")
        for k in ["sleepA", "sleepI", "sleepS", "sleepM", "sleepL", "wakeI"]:
            w(f"""          {k}: >-
            {{%- set m = trigger.payload.split(' ERREUR')[0] | regex_findall('{k}:(\\d+)') -%}}
            {{{{ m[0] | int if m else none }}}}""")
        w(f"""          erreur: >-
            {{%- set p = trigger.payload.split(' ERREUR ') -%}}
            {{{{ p[1] | regex_replace(' \\d{{1,3}}(\\.\\d{{1,3}}){{3}}$', '') if p | length > 1 else '' }}}}

  # ---------- {nom} : réponse STATUS ----------
  # LoRa : une ligne « {nom} ETAT <date> <heure> | Wdog : ... | Uptime : ...s <IP> »
  # Wi-Fi : plusieurs lignes « === ÉTAT {nom} === », date, IP, Wdog, Uptime (remises sur une ligne, séparées par « | »)
  - trigger:
      - trigger: mqtt
        topic: "home/conf{i}/state"
        value_template: >-
          {{{{ 'ETAT' if (value.split(' ') + [''])[1] | upper in ['ETAT', 'ÉTAT'] else '' }}}}
        payload: "ETAT"
    sensor:
      - name: "{nom} Rapport"
        unique_id: {i}_rapport
        icon: mdi:information-outline
        state: >-
          {{%- set p = trigger.payload -%}}
          {{%- if p.startswith('===') -%}}
            {{%- set s = p.split('\\n')[1:] | map('trim') | reject('eq', '') | join(' | ') -%}}
          {{%- else -%}}
            {{%- set s = p.split(' ', 2)[2] -%}}
          {{%- endif -%}}
          {{{{ (s | regex_replace(' {{2,}}', ' ') | regex_replace(' \\d{{1,3}}(\\.\\d{{1,3}}){{3}}$', ''))[:255] }}}}
        attributes:
          recu: "{{{{ now().isoformat() }}}}"

  # ---------- {nom} : activité (éveillé jusqu'à DEEPSLEEP ON ; auto_off si la trame de coucher est perdue) ----------
  - trigger:""")
        for u in n["unites"]:
            w(f"""      - trigger: mqtt
        topic: "home/thermometre/state"
        value_template: "{{{{ value.split(' ')[0] | upper }}}}"
        payload: "{(u + n['suffixe']).upper()}\"""")
        w(f"""      - trigger: mqtt
        topic: "home/conf{i}/state"
    binary_sensor:
      - name: "{nom} Éveillé"
        unique_id: {i}_eveille
        icon: mdi:sleep-off
        state: >-
          {{%- set p = trigger.payload | upper -%}}
          {{{{ 'DEEPSLEEP ON' not in p and not p.endswith(' OFFLINE') }}}}
        auto_off: "00:01:00"
    sensor:
      - name: "{nom} Dernière trame"
        unique_id: {i}_derniere_trame
        device_class: timestamp
        icon: mdi:access-point
        state: "{{{{ now().isoformat() }}}}"
""")
    w("input_boolean:")
    for n in NOEUDS:
        w(f"""  # ---------- {n['nom']} ----------
  {n['id']}_sleepa:
    name: "{n['nom']} Deep sleep actif"
    icon: mdi:sleep
""")
    w("input_number:")
    for n in NOEUDS:
        w(f"  # ---------- {n['nom']} (bornes du capteur : 1 à 86400 s, wakeI 1 à 3600 s) ----------")
        for k, lib, mx, ic in PARAMS:
            w(f"""  {n['id']}_{k}:
    name: "{n['nom']} {lib}"
    min: 1
    max: {mx}
    step: 1
    mode: box
    unit_of_measurement: "s"
    icon: {ic}""")
        w("")
    w("input_text:")
    for n in NOEUDS:
        w(f"""  # ---------- {n['nom']} ----------
  {n['id']}_commande:
    name: "{n['nom']} Commande libre"
    max: 100
    icon: mdi:console
""")
    w(f"""script:
  # ------------------------------------------------------------
  # Envoie une commande au nœud.
  # LoRa : via la file d'attente du relais (envoyée au prochain réveil ; ANNULER est traité par le relais).
  # Wi-Fi : publiée en RETENU (le nœud la lit à son réveil) ; effacée par l'automatisation
  #         « Nœuds Wi-Fi - Effacer la commande retenue » ; ANNULER = effacement du message retenu.
  # Refusée si une commande attend déjà (sauf ANNULER) : une nouvelle commande remplacerait celle qui attend.
  # ------------------------------------------------------------
  noeud_commande:
    alias: Nœud capteur - Envoyer une commande
    icon: mdi:send
    mode: queued
    fields:
      capteur:
        name: Nœud
        description: Identifiant du nœud (thremise, thchrdc...)
        required: true
        example: thchrdc
        selector:
          text:
      commande:
        name: Commande
        description: GET, STATUS, REBOOT, ANNULER, SET cle:valeur..., SLEEP..., WDOG...
        required: true
        example: STATUS
        selector:
          text:
    sequence:
      - variables:
          # Nœuds LoRa (les autres sont en Wi-Fi) : même liste que dans genere_noeuds.py
          lora: "{liste_yaml(LORA)}"
          wifi: "{{{{ capteur not in lora }}}}"
          cmd: "{{{{ commande | trim }}}}"
          attente: "{{{{ states('sensor.' ~ capteur ~ '_commande_en_attente') }}}}"
      - if:
          - condition: template
            value_template: "{{{{ cmd == '' }}}}"
        then:
          - stop: "Commande vide"
      - if:
          - condition: template
            value_template: >-
              {{{{ cmd | upper != 'ANNULER' and attente not in ['aucune', 'unknown', 'unavailable', ''] }}}}
        then:
          - action: persistent_notification.create
            data:
              title: "Nœud {{{{ capteur }}}}"
              message: >-
                Commande « {{{{ cmd }}}} » refusée : « {{{{ attente }}}} » est déjà en attente.
                Annulez-la d'abord, ou attendez le prochain réveil du nœud.
          - stop: "Une commande est déjà en attente"
      - action: mqtt.publish
        data:
          topic: "home/conf{{{{ capteur }}}}/command"
          payload: "{{{{ '' if wifi and cmd | upper == 'ANNULER' else cmd }}}}"
          retain: "{{{{ wifi }}}}"

  # ------------------------------------------------------------
  # Efface le message retenu du topic command (commande restée en retenu, ancien testament « <nom> offline »...)
  # Sans effet sur la file du relais (il ignore un message vide).
  # ------------------------------------------------------------
  noeud_nettoyer_retenu:
    alias: Nœud capteur - Nettoyer le message retenu
    icon: mdi:broom
    mode: queued
    fields:
      capteur:
        name: Nœud
        description: Identifiant du nœud (thremise, thchrdc...)
        required: true
        example: thchrdc
        selector:
          text:
    sequence:
      - action: mqtt.publish
        data:
          topic: "home/conf{{{{ capteur }}}}/command"
          payload: ""
          retain: true

  # ------------------------------------------------------------
  # Un seul SET avec les réglages qui diffèrent de la dernière config reçue
  # ------------------------------------------------------------
  noeud_enregistrer:
    alias: Nœud capteur - Enregistrer les réglages
    icon: mdi:content-save
    mode: queued
    fields:
      capteur:
        name: Nœud
        description: Identifiant du nœud (thremise, thchrdc...)
        required: true
        example: thchrdc
        selector:
          text:
    sequence:
      - variables:
          cfg: "sensor.{{{{ capteur }}}}_config"
          params: >-
            {{%- set ns = namespace(s='') -%}}
            {{%- set a = 1 if is_state('input_boolean.' ~ capteur ~ '_sleepa', 'on') else 0 -%}}
            {{%- if a != state_attr(cfg, 'sleepA') -%}}{{%- set ns.s = ns.s ~ ' sleepA:' ~ a -%}}{{%- endif -%}}
            {{%- for k in ['sleepI', 'sleepS', 'sleepM', 'sleepL', 'wakeI'] -%}}
              {{%- set v = states('input_number.' ~ capteur ~ '_' ~ k | lower) | float(0) | int -%}}
              {{%- if v > 0 and v != state_attr(cfg, k) -%}}{{%- set ns.s = ns.s ~ ' ' ~ k ~ ':' ~ v -%}}{{%- endif -%}}
            {{%- endfor -%}}
            {{{{- ns.s | trim -}}}}
          # Réglages trop courts (faute de frappe, champs restés au minimum) : 10 s au moins
          trop_courts: >-
            {{%- set ns = namespace(l=[]) -%}}
            {{%- for k in ['sleepI', 'sleepS', 'sleepM', 'sleepL', 'wakeI'] -%}}
              {{%- if states('input_number.' ~ capteur ~ '_' ~ k | lower) | float(0) < 10 -%}}{{%- set ns.l = ns.l + [k] -%}}{{%- endif -%}}
            {{%- endfor -%}}
            {{{{- ns.l | join(', ') -}}}}
      # Sans config reçue, les champs ne reflètent pas le nœud (valeur minimale à la création) : envoi refusé
      - if:
          - condition: template
            value_template: "{{{{ state_attr(cfg, 'sleepI') is none }}}}"
        then:
          - action: persistent_notification.create
            data:
              title: "Nœud {{{{ capteur }}}}"
              message: >-
                Config du nœud inconnue : cliquez d'abord sur « Relire » et attendez son réveil,
                les réglages se rempliront avec ses vraies valeurs.
          - stop: "Config inconnue"
      - if:
          - condition: template
            value_template: "{{{{ trop_courts != '' }}}}"
        then:
          - action: persistent_notification.create
            data:
              title: "Nœud {{{{ capteur }}}}"
              message: "Envoi refusé : durée inférieure à 10 s pour {{{{ trop_courts }}}}."
          - stop: "Durée trop courte"
      - if:
          - condition: template
            value_template: "{{{{ params == '' }}}}"
        then:
          - action: persistent_notification.create
            data:
              title: "Nœud {{{{ capteur }}}}"
              message: "Aucun réglage modifié : rien à envoyer."
          - stop: "Aucune modification"
      - action: script.noeud_commande
        data:
          capteur: "{{{{ capteur }}}}"
          commande: "SET {{{{ params }}}}"

  # ------------------------------------------------------------
  # Envoie la commande libre saisie dans input_text.<nœud>_commande, puis vide le champ
  # ------------------------------------------------------------
  noeud_commande_libre:
    alias: Nœud capteur - Envoyer la commande libre
    icon: mdi:console
    mode: queued
    fields:
      capteur:
        name: Nœud
        description: Identifiant du nœud (thremise, thchrdc...)
        required: true
        example: thchrdc
        selector:
          text:
    sequence:
      - variables:
          cmd: "{{{{ states('input_text.' ~ capteur ~ '_commande') }}}}"
      - action: script.noeud_commande
        data:
          capteur: "{{{{ capteur }}}}"
          commande: "{{{{ cmd }}}}"
      - action: input_text.set_value
        target:
          entity_id: "input_text.{{{{ capteur }}}}_commande"
        data:
          value: ""

automation:
  # ------------------------------------------------------------
  # Config reçue du nœud : recopiée dans les réglages (input_boolean / input_number)
  # ------------------------------------------------------------
  - id: capteurs_lora_synchro_reglages
    alias: Nœuds capteurs - Recopier la config reçue dans les réglages
    mode: queued
    triggers:
      - trigger: state
        entity_id:""")
    for n in NOEUDS:
        w(f"          - sensor.{n['id']}_config")
    w(f"""    actions:
      - variables:
          capteur: "{{{{ trigger.entity_id.split('.')[1] | replace('_config', '') }}}}"
      - if:
          - condition: template
            value_template: "{{{{ state_attr(trigger.entity_id, 'sleepA') is not none }}}}"
        then:
          - action: >-
              input_boolean.turn_{{{{ 'on' if state_attr(trigger.entity_id, 'sleepA') == 1 else 'off' }}}}
            target:
              entity_id: "input_boolean.{{{{ capteur }}}}_sleepa"
      - repeat:
          for_each: ["sleepI", "sleepS", "sleepM", "sleepL", "wakeI"]
          sequence:
            - if:
                - condition: template
                  value_template: "{{{{ state_attr(trigger.entity_id, repeat.item) is number }}}}"
              then:
                - action: input_number.set_value
                  target:
                    entity_id: "input_number.{{{{ capteur }}}}_{{{{ repeat.item | lower }}}}"
                  data:
                    value: "{{{{ state_attr(trigger.entity_id, repeat.item) }}}}"

  # ------------------------------------------------------------
  # Nœuds Wi-Fi : efface la commande retenue dès que le nœud l'a reçue, c.-à-d. à son accusé
  # (« <nom> COMMANDE reçue ... ») ou à sa connexion (« <nom> online le ... », publié APRÈS ses abonnements :
  # la commande retenue lui est déjà remise). Évite qu'elle soit rejouée à chaque réveil (REBOOT en boucle).
  # ------------------------------------------------------------
  - id: noeuds_wifi_effacer_commande_retenue
    alias: Nœuds Wi-Fi - Effacer la commande retenue
    mode: queued
    triggers:
      # home/+/state couvre home/thermometre/state (« online ») et home/conf<id>/state (accusé) ;
      # filtré dans le déclencheur pour ne pas lancer l'automatisation à chaque mesure
      - trigger: mqtt
        topic: "home/+/state"
        value_template: >-
          {{{{ 'oui' if (' COMMANDE reçue ' in value or ' online le ' in value) else 'non' }}}}
        payload: "oui"
    variables:
      # Nœuds Wi-Fi : même liste que dans genere_noeuds.py
      wifi: "{liste_yaml(WIFI)}"
      capteur: "{{{{ (trigger.payload.split(' ') + [''])[0] | lower }}}}"
    conditions:
      - condition: template
        value_template: >-
          {{{{ capteur in wifi
             and states('sensor.' ~ capteur ~ '_commande_en_attente') not in ['aucune', 'unknown', 'unavailable', ''] }}}}
    actions:
      - action: mqtt.publish
        data:
          topic: "home/conf{{{{ capteur }}}}/command"
          payload: ""
          retain: true
""")
    return "\n".join(o)


# ============================================================ VUE
def bouton(nom, icone, script, i, commande=None, confirmation=None, rouge=False, indent=14):
    p = " " * indent
    s = [f"{p}- type: button", f"{p}  name: {nom}", f"{p}  icon: {icone}", f"{p}  tap_action:",
         f"{p}    action: perform-action", f"{p}    perform_action: script.{script}", f"{p}    data:",
         f"{p}      capteur: {i}"]
    if commande:
        s.append(f"{p}      commande: {commande}")
    if confirmation:
        s += [f"{p}    confirmation:", f"{p}      text: {confirmation}"]
    if rouge:
        s += [f"{p}  card_mod:", f"{p}    style: |", f"{p}      ha-card {{", f"{p}        height: 60px !important;",
              f"{p}        background: rgba(200,0,0,0.2) !important;", f"{p}      }}",
              f"{p}      ha-icon {{ color: red !important; }}"]
    else:
        s += [f"{p}  card_mod:", f'{p}    style: "ha-card {{ height: 60px !important; }}"']
    return "\n".join(s)


def vue():
    o = []
    w = o.append
    w("""# Vue « Nœuds capteurs » du tableau Système (https://ha.jbil.org/dashboard-systeme/noeuds-capteurs)
# Généré par genere_noeuds.py (entités et scripts : packages/noeuds_capteurs.yaml).
# Installation : tableau Système → ✏️ → vue « Capteurs LoRa » (ou « + » nouvelle vue) → ⋮ → « Modifier en YAML »
# → remplacer par ce fichier → Enregistrer.
type: sections
max_columns: 4
title: Nœuds capteurs
path: noeuds-capteurs
icon: mdi:access-point-network
show_icon_and_title: true
sections:""")
    for n in NOEUDS:
        i, nom, wifi = n["id"], n["nom"], n["mode"] == "wifi"
        icone = "mdi:wifi" if wifi else "mdi:radio-tower"
        w(f"""  # ======================= {nom} ({'Wi-Fi' if wifi else 'LoRa'}) =======================
  - type: grid
    cards:
      - type: vertical-stack
        cards:
          - type: heading
            heading: {nom} · {'Wi-Fi' if wifi else 'LoRa'}
            heading_style: title
            icon: {icone}
          - type: entities
            entities:
              - entity: binary_sensor.{i}_eveille
                name: Éveillé
              - entity: sensor.{i}_derniere_trame
                name: Dernière trame
                format: relative
              - entity: sensor.{i}_commande_en_attente
                name: Commande en attente
              - entity: sensor.{i}_config
                name: Config reçue
                format: relative""")
        if n["mesures"]:
            w("""          - type: entities
            title: Mesures
            entities:""")
            for e, lib in n["mesures"]:
                w(f"""              - entity: {e}
                name: {lib}""")
        w(f"""          - type: markdown
            content: |""")
        if wifi:
            w(f"""              {{% set ip = states('input_text.ip_{i}') %}}
              {{% if ip not in ['unknown', 'unavailable', ''] %}}
              **Page Web** : [{{{{ ip }}}}](http://{{{{ ip }}}}/)

              {{% endif %}}""")
        w(f"""              {{% set r = states('sensor.{i}_rapport') %}}
              {{% if r not in ['unknown', 'unavailable'] %}}
              **État** (reçu {{{{ relative_time(as_datetime(state_attr('sensor.{i}_rapport', 'recu'))) }}}}) :

              {{{{ r | replace(' | ', '  \\n') }}}}
              {{% else %}}
              *Pas encore de réponse au bouton État.*
              {{% endif %}}
              {{% set e = state_attr('sensor.{i}_config', 'erreur') %}}
              {{% if e %}}

              ⚠️ **Refusé par le nœud** : {{{{ e }}}}
              {{% endif %}}
          - type: horizontal-stack
            cards:""")
        w(bouton("Relire", "mdi:refresh", "noeud_commande", i, "GET", indent=14))
        w(bouton("État", "mdi:information", "noeud_commande", i, "STATUS", indent=14))
        w(bouton("Annuler", "mdi:cancel", "noeud_commande", i, "ANNULER", indent=14))
        w(bouton("Redémarrer", "mdi:restart", "noeud_commande", i, "REBOOT",
                 confirmation=f"Redémarrer {nom} à son prochain réveil ?", rouge=True, indent=14))
        w(f"""      - type: vertical-stack
        cards:
          - type: entities
            title: Réglages (envoyés au prochain réveil)
            entities:
              - entity: input_boolean.{i}_sleepa
                name: Deep sleep actif""")
        for k, lib, _, _ in PARAMS:
            w(f"""              - entity: input_number.{i}_{k}
                name: {lib}""")
        w(f"""          - type: button
            name: Enregistrer
            icon: mdi:content-save
            tap_action:
              action: perform-action
              perform_action: script.noeud_enregistrer
              data:
                capteur: {i}
            # Grisé et non cliquable tant qu'aucune config n'a été reçue du nœud (le script refuse aussi)
            card_mod:
              style: |
                ha-card {{
                  height: 60px !important;
                  {{% if states('sensor.{i}_config') in ['unknown', 'unavailable'] %}}
                  opacity: 0.4;
                  pointer-events: none;
                  {{% endif %}}
                }}
          - type: entities
            title: Commande libre
            entities:
              - entity: input_text.{i}_commande
                name: Commande
          - type: horizontal-stack
            cards:""")
        w(bouton("Envoyer", "mdi:send", "noeud_commande_libre", i, indent=14))
        w(bouton("Nettoyer retenu", "mdi:broom", "noeud_nettoyer_retenu", i,
                 confirmation=f"Effacer le message retenu sur home/conf{i}/command ?", indent=14))
        w("")
    w("""  # ======================= Aide =======================
  - type: grid
    cards:
      - type: markdown
        title: Commandes d'un nœud capteur
        content: |
          Casse indifférente, durées en secondes. Une commande part au **prochain réveil** du nœud
          (LoRa : file d'attente du relais ; Wi-Fi : message retenu, effacé dès que le nœud l'a reçu) ;
          une seule à la fois : annuler d'abord pour en changer.

          | Commande | Effet |
          |---|---|
          | `GET` | Relit la configuration |
          | `SET cle:valeur ...` | Modifie des paramètres : `sleepA` 0/1, `sleepI` `sleepS` `sleepM` `sleepL` (1 à 86400), `wakeI` (1 à 3600) |
          | `SLEEP ENABLE` / `SLEEP DISABLE` | Active / désactive le deep sleep |
          | `SLEEP n` | Intervalle = n s, et active le deep sleep |
          | `SLEEPS` / `SLEEPM` / `SLEEPL` | Applique la durée courte / moyenne / longue |
          | `SLEEPS n` (idem M, L) | Règle la durée courte (moyenne, longue) sans l'appliquer |
          | `WDOG ENABLE` / `WDOG DISABLE` / `WDOG s` | Watchdog ALIVE (ignoré en deep sleep) |
          | `STATUS` | Rapport d'état (bouton État) |
          | `REBOOT` | Redémarre le nœud |
          | `ANNULER` | Vide la commande en attente (LoRa : traitée par le relais ; Wi-Fi : efface le message retenu) |

          **Nettoyer retenu** : efface le message retenu du topic `home/conf<nœud>/command` (commande restée en
          retenu, ancien testament « offline »…). Sans effet sur la file du relais.

          L'automatisation « Cuve - SLEEP et ACK flotteur » n'envoie pas son SLEEP quand une commande attend.

          Page du relais (réseau local) : [Capteurs LoRa](http://{{ states('input_text.ip_relaislora') }}/capteurs)
""")
    return "\n".join(o)


def ecrit(chemin, texte):
    with open(chemin, "w", encoding="utf-8", newline="\r\n") as f:
        f.write(texte)
    print("écrit :", chemin)


ecrit(os.path.join(HA, "packages", "noeuds_capteurs.yaml"), paquet())
ecrit(os.path.join(HA, "vue-noeuds-capteurs.yaml"), vue())
