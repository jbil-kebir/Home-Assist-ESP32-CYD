# -*- coding: utf-8 -*-
# Génère HA/packages/noeuds_capteurs.yaml et HA/vue-noeuds-capteurs.yaml à partir de la liste des nœuds.
# Lancer depuis n'importe où : python -I HA/genere_noeuds.py (écrit dans le dossier du script)
import os

HA = os.path.dirname(os.path.abspath(__file__))  # dossier HA du projet

# id = th<suffixe> (topics home/conf<id>/...) ; nom = nom de l'équipement ; mode = lora / wifi
# unites = préfixes des capteurs unitaires du nœud (trames « <Prefixe><Suffixe> ... » sur home/thermometre/state)
# trames (facultatif) = [(topic, NOM)] explicites, à la place de unites (nœud qui ne suit pas la règle du suffixe)
# mesures = (entité, nom affiché) du bloc Mesures (entités existantes de configuration.yaml ou des paquets)
# ip (facultatif) = entité qui contient l'IP du nœud Wi-Fi (lien vers sa page Web)
# jn (facultatif) = nœud équipé du détecteur jour / nuit (CJourNuit) : attributs jn_* de sa config, affichés en lecture
#   seule (modifiables par la commande libre : SET jn_sbas ...)
# fl (facultatif) = nœud équipé du flotteur avec réveil et confirmation (Capteur 3.1) : attributs mfv_* de sa config,
#   affichés en lecture seule (modifiables par la commande libre : SET mfv_revmin ...)
# vue (facultatif, True par défaut) = False : nœud absent de la vue Nœuds capteurs (entités et scripts gardés)
# sans_retenu (facultatif) = nœud Wi-Fi toujours éveillé qui ne publie pas « online » (pas de DHT20) : commandes non
#   retenues, sinon un REBOOT dont l'accusé se perd serait rejoué en boucle (commande perdue s'il est hors tension)
NOEUDS = [
    dict(id="thremise", nom="ThRemise", suffixe="Remise", mode="lora", unites=["Th", "Bat", "Flotteur"], fl=True,
         mesures=[("sensor.thremise_temperature", "Température"), ("sensor.thremise_humidite", "Humidité"),
                  ("sensor.batremise_tension", "Batterie"), ("sensor.batremise_etat", "État batterie"),
                  ("sensor.flotteur_remise", "Flotteur")]),
    # ThNomade : pas encore d'entités de mesure dans configuration.yaml ; vue=False : retiré de la vue Nœuds capteurs
    # dans HA par l'utilisateur (export du 31/10/2026), entités et scripts gardés
    dict(id="thnomade", nom="ThNomade", suffixe="Nomade", mode="lora", unites=["Th", "Bat", "Flotteur"], mesures=[],
         vue=False),
    dict(id="thcave", nom="ThCave", suffixe="Cave", mode="lora", unites=["Th", "Bat", "Jn"], jn=True,
         mesures=[("sensor.thcave_temperature", "Température"), ("sensor.thcave_humidite", "Humidité"),
                  ("sensor.batcave_tension", "Batterie"), ("sensor.batcave_etat", "État batterie"),
                  ("sensor.jncave_etat", "Jour / nuit"), ("sensor.jncave_eclairement", "Éclairement")]),
    dict(id="thchrdc", nom="ThChRDC", suffixe="ChRDC", mode="wifi", unites=["Th", "Bat"], ip="input_text.ip_thchrdc",
         mesures=[("sensor.thchrdc_temperature", "Température"), ("sensor.thchrdc_humidite", "Humidité"),
                  ("sensor.batchrdc_tension", "Batterie"), ("sensor.batchrdc_etat", "État batterie")]),
    dict(id="thch1er", nom="ThCh1er", suffixe="Ch1er", mode="wifi", unites=["Th", "Bat"], ip="input_text.ip_thch1er",
         mesures=[("sensor.thch1er_temperature", "Température"), ("sensor.thch1er_humidite", "Humidité"),
                  ("sensor.batch1er_tension", "Batterie"), ("sensor.batch1er_etat", "État batterie")]),
    # Capteur chaudière Sdb : S3 laissé en Wi-Fi (nœud critique ; basculer Wi-Fi / LoRa à terme)
    dict(id="thsdb", nom="ThSdb", suffixe="Sdb", mode="wifi", unites=["Th", "Bat", "Coul"], ip="input_text.ip_thsdb",
         mesures=[("sensor.thsdb_temperature", "Température"), ("sensor.thsdb_humidite", "Humidité"),
                  ("sensor.coulsdb_del", "Voyant chaudière"), ("sensor.coulsdb_rouge", "Rouge"),
                  ("sensor.coulsdb_vert", "Vert"), ("sensor.coulsdb_bleu", "Bleu"),
                  ("sensor.coulsdb_intensite", "Intensité"),
                  ("sensor.batsdb_tension", "Batterie"), ("sensor.batsdb_etat", "État batterie")]),
    # Chauffage SdB : ACTIONNEUR, pas un capteur (C3 alimenté par le relais 433 MHz quand le chauffage marche ; envoie l'IR
    # au démarrage et sur ordre Wi-Fi, micro pour ses besoins locaux) : hors règle du suffixe (nœud « Noeud », trames
    # « NoeudChauffageSb » sur home/noeud/state). Pas de réglages de sommeil ; carte à part pour la vue Accueil → Actionneurs
    # (présence, RSSI et bouton du chauffage y sont déjà : voir packages/chauffage.yaml).
    dict(id="noeud", nom="Noeud", titre="Nœud chauffage SdB", type="actionneur", suffixe="", mode="wifi", unites=[],
         trames=[("home/noeud/state", "NOEUDCHAUFFAGESB")], ip="sensor.c3_chauffage_sdb_ip", sans_retenu=True,
         mesures=[]),
]
CAPTEURS = [n for n in NOEUDS if n.get("type") != "actionneur"]
ACTIONNEURS = [n for n in NOEUDS if n.get("type") == "actionneur"]
LORA = [n["id"] for n in NOEUDS if n["mode"] == "lora"]
WIFI = [n["id"] for n in NOEUDS if n["mode"] == "wifi"]
SANS_RETENU = [n["id"] for n in NOEUDS if n.get("sans_retenu")]


def trames(n):
    return n.get("trames") or [("home/thermometre/state", (u + n["suffixe"]).upper()) for u in n["unites"]]
# Réglages du détecteur jour / nuit (attributs de sensor.<id>_config, lecture seule) : (clé, libellé, unité)
PARAMS_JN = [("jn_sbas", "Seuil bas jour → nuit", "%"), ("jn_shaut", "Seuil haut nuit → jour", "%"),
             ("jn_ecl", "Remonter l'éclairement (1 oui, 0 non)", ""), ("jn_inter", "Intervalle de mesure", "s"),
             ("jn_force", "Délai max sans envoi", "s")]
# Réglages du flotteur (Capteur 3.1, attributs de sensor.<id>_config, lecture seule) : (clé, libellé, unité)
PARAMS_FL = [("mfv_rev", "Réveil par le flotteur (1 oui, 0 non)", ""), ("mfv_revmin", "Anti-rafale", "s"),
             ("mfv_ack", "Confirmation par le contrôleur (1 oui, 0 non)", ""), ("mfv_ackn", "Essais de confirmation", "")]
PARAMS = [("sleepi", "Intervalle deep sleep", 86400, "mdi:timer-outline"),
          ("sleeps", "Sommeil court (SLEEPS)", 86400, "mdi:timer-outline"),
          ("sleepm", "Sommeil moyen (SLEEPM)", 86400, "mdi:timer-outline"),
          ("sleepl", "Sommeil long (SLEEPL)", 86400, "mdi:timer-outline"),
          ("wakei", "Éveil maximum", 3600, "mdi:timer-sand")]


def liste_yaml(l):
    return "[" + ", ".join("'%s'" % x for x in l) + "]"


# ============================================================ PAQUET
# Version du firmware : « <nom> VERSION 3.4 31/10/2026 13:13 [IP] » sur le topic de configuration (Capteur 3.4,
# Relais-Lora 2.3, Home-Assist 3.5 : voir packages/cyd.yaml). État = numéro de version ; attributs : compilation
# (date et heure), recu (dernière réception). Gardé au redémarrage de HA (template à déclencheur).
def capteur_version(nom, i, topic, quand):
    return f"""  # ---------- {nom} : version du firmware ({quand}) ----------
  - trigger:
      - trigger: mqtt
        topic: "{topic}"
    conditions:
      - condition: template
        value_template: "{{{{ ' VERSION ' in trigger.payload }}}}"
    sensor:
      - name: "{nom} Version"
        unique_id: {i}_version
        icon: mdi:chip
        state: "{{{{ trigger.payload.split(' VERSION ')[1].split(' ')[0] }}}}"
        attributes:
          compilation: >-
            {{%- set w = trigger.payload.split(' VERSION ')[1].split(' ') -%}}
            {{{{ (w[1] ~ ' ' ~ w[2]) if w | count > 2 else '' }}}}
          recu: "{{{{ now().isoformat() }}}}"
"""


# Relais LoRa : signe de vie = « <nom> ALIVE <date> <heure> » toutes les 60 s (watchdog ALIVE, Relais-Lora 2.3) et toute
# autre trame du relais (sur son topic, annonce DEVICE RELAIS LORA, trames LoRa relayées : elles finissent par « 0.0.0.0 »).
# Connecté = dernier signal de moins de RELAIS_DELAI_ALERTE_S (3 périodes ALIVE) et pas de testament plus récent
# (« <nom> - Reconnect le ... », publié par le broker quand la connexion est perdue). Recalculé chaque minute (now()).
# sensor.relaislora_etat : réponse au STATUS (attribut « texte »).
RELAIS_DELAI_ALERTE_S = 180
ENTITES_RELAIS = """  # ---------- Relais LoRa : connexion (dernier signal récent, pas de testament plus récent) ----------
  - binary_sensor:
      - name: "RelaisLora Connexion"
        unique_id: relaislora_connexion
        device_class: connectivity
        state: >-
          {%- if has_value('sensor.relaislora_dernier_signal') -%}
            {%- set d = as_datetime(states('sensor.relaislora_dernier_signal')) -%}
            {%- set t = as_datetime(states('sensor.relaislora_testament')) if has_value('sensor.relaislora_testament') else none -%}
            {{ (now() - d).total_seconds() < """ + str(RELAIS_DELAI_ALERTE_S) + """ and (t is none or t < d) }}
          {%- else -%}
            false
          {%- endif -%}

  # ---------- Relais LoRa : testament (connexion MQTT perdue) ----------
  - trigger:
      - trigger: mqtt
        topic: "home/confrelaislora/state"
    conditions:
      - condition: template
        value_template: "{{ ' - Reconnect le ' in trigger.payload }}"
    sensor:
      - name: "RelaisLora Testament"
        unique_id: relaislora_testament
        device_class: timestamp
        icon: mdi:lan-disconnect
        state: "{{ now().isoformat() }}"

  # ---------- Relais LoRa : dernier signal (ALIVE, toute autre trame du relais, trame LoRa relayée) ----------
  - trigger:
      - trigger: mqtt
        topic: "home/confrelaislora/state"
      - trigger: mqtt
        topic: "home/configuration/command"
        value_template: "{{ 'oui' if value.upper().startswith('DEVICE RELAIS LORA') else 'non' }}"
        payload: "oui"
      - trigger: mqtt
        topic: "home/thermometre/state"
        value_template: "{{ 'oui' if value.endswith(' 0.0.0.0') else 'non' }}"
        payload: "oui"
    conditions:
      - condition: template
        value_template: "{{ ' - Reconnect le ' not in trigger.payload }}"
    sensor:
      - name: "RelaisLora Dernier signal"
        unique_id: relaislora_dernier_signal
        device_class: timestamp
        icon: mdi:access-point
        state: "{{ now().isoformat() }}"

  # ---------- Relais LoRa : réponse au STATUS (« === ÉTAT <nom> === », version, IP, uptime, files d'attente) ----------
  - trigger:
      - trigger: mqtt
        topic: "home/confrelaislora/state"
    conditions:
      - condition: template
        value_template: "{{ trigger.payload.startswith('=== ') }}"
    sensor:
      - name: "RelaisLora État"
        unique_id: relaislora_etat
        device_class: timestamp
        icon: mdi:router-wireless
        state: "{{ now().isoformat() }}"
        attributes:
          texte: "{{ trigger.payload }}"
"""


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
#           sensor.<id>_version (firmware, trame « <nom> VERSION 3.4 31/10/2026 13:13 » : Capteur 3.4),
#           sensor.relaislora_version (Relais-Lora 2.3), binary_sensor.relaislora_connexion,
#           sensor.relaislora_dernier_signal, sensor.relaislora_testament, sensor.relaislora_etat (réponse STATUS,
#           attribut texte) : vue Relais LoRa ; connexion = signal de moins de 3 min (ALIVE toutes les 60 s)
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
            w(f"""  # ---------- {nom} : commande en attente = message retenu sur le topic command ----------
  # Vide, ou ancien testament retenu « <nom> offline » (ancien firmware) : « aucune »
  - trigger:
      - trigger: mqtt
        topic: "home/conf{i}/command"
    sensor:
      - name: "{nom} Commande en attente"
        unique_id: {i}_commande_attente
        icon: mdi:timer-sand
        state: >-
          {{%- set p = trigger.payload | trim -%}}
          {{{{ 'aucune' if p == '' or p | lower is search(' offline$') else p[:255] }}}}
""")
        # Actionneur : pas de config de sommeil ; le sensor Config est créé quand même (inutilisé, sans effet)
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
        # mute : absent de la réponse d'un firmware antérieur au 28/10/2026 → none
        # jn_* : nœud équipé du détecteur jour / nuit seulement ; mfv_* : nœud équipé du flotteur (fl) seulement
        for k in (["sleepA", "sleepI", "sleepS", "sleepM", "sleepL", "wakeI", "mute"]
                  + ([c for c, _, _ in PARAMS_JN] if n.get("jn") else [])
                  + ([c for c, _, _ in PARAMS_FL] if n.get("fl") else [])):
            w(f"""          {k}: >-
            {{%- set m = trigger.payload.split(' ERREUR')[0] | regex_findall('{k}:(\\d+)') -%}}
            {{{{ m[0] | int if m else none }}}}""")
        w(f"""          erreur: >-
            {{%- set p = trigger.payload.split(' ERREUR ') -%}}
            {{{{ p[1] | regex_replace(' \\d{{1,3}}(\\.\\d{{1,3}}){{3}}$', '') if p | length > 1 else '' }}}}

  # ---------- {nom} : réponse STATUS ----------
  # Wi-Fi et LoRa : une ligne « {nom} ETAT <date> <heure> | [IP : <ip> | ]Wdog : ... | Uptime : ...s <IP> »
  # Ancien firmware Wi-Fi (avant Capteur 2.1) : plusieurs lignes « === ÉTAT {nom} === » (remises sur une ligne, séparées par « | »)
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

  # ---------- {nom} : activité (éveillé jusqu'à DEEPSLEEP ON, sans délai : même règle que les CYD) ----------
  - trigger:""")
        for topic, nom_trame in trames(n):
            w(f"""      - trigger: mqtt
        topic: "{topic}"
        value_template: "{{{{ value.split(' ')[0] | upper }}}}"
        payload: "{nom_trame}\"""")
        w(f"""      - trigger: mqtt
        topic: "home/conf{i}/state"
    binary_sensor:
      - name: "{nom} Éveillé"
        unique_id: {i}_eveille
        icon: mdi:sleep-off
        state: >-
          {{%- set p = trigger.payload | upper -%}}
          {{{{ 'DEEPSLEEP ON' not in p and not p.endswith(' OFFLINE') }}}}
    sensor:
      - name: "{nom} Dernière trame"
        unique_id: {i}_derniere_trame
        device_class: timestamp
        icon: mdi:access-point
        state: "{{{{ now().isoformat() }}}}"
""")
        w(capteur_version(nom, i, f"home/conf{i}/state", "au démarrage, pas au réveil d'un deep sleep, et après GET"))
    w(capteur_version("RelaisLora", "relaislora", "home/confrelaislora/state", "au démarrage et après STATUS"))
    w(ENTITES_RELAIS)
    w("input_boolean:")
    for n in CAPTEURS:
        w(f"""  # ---------- {n['nom']} ----------
  {n['id']}_sleepa:
    name: "{n['nom']} Deep sleep actif"
    icon: mdi:sleep
  {n['id']}_mute:
    name: "{n['nom']} Mesures muettes"
    icon: mdi:volume-off
""")
    w("input_number:")
    for n in CAPTEURS:
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
  #         Nœuds « sans_retenu » (toujours éveillés, sans « online ») : publiée sans retenu.
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
          # Nœuds Wi-Fi toujours éveillés sans « online » : commande non retenue (même liste que dans genere_noeuds.py)
          sans_retenu: "{liste_yaml(SANS_RETENU)}"
          retenu: "{{{{ wifi and capteur not in sans_retenu }}}}"
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
          retain: "{{{{ retenu }}}}"

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
            {{#- mute : seulement si le firmware le connaît (présent dans la dernière config reçue) -#}}
            {{%- set m = 1 if is_state('input_boolean.' ~ capteur ~ '_mute', 'on') else 0 -%}}
            {{%- if state_attr(cfg, 'mute') is not none and m != state_attr(cfg, 'mute') -%}}{{%- set ns.s = ns.s ~ ' mute:' ~ m -%}}{{%- endif -%}}
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
    for n in CAPTEURS:
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
      - if:
          - condition: template
            value_template: "{{{{ state_attr(trigger.entity_id, 'mute') is not none }}}}"
        then:
          - action: >-
              input_boolean.turn_{{{{ 'on' if state_attr(trigger.entity_id, 'mute') == 1 else 'off' }}}}
            target:
              entity_id: "input_boolean.{{{{ capteur }}}}_mute"
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


# Teinte de fond de chaque nœud (cartes entities et markdown de sa section, pas les boutons), pour les distinguer ;
# transparence faible : lisible en thème clair comme en thème sombre
FONDS_NOEUDS = ["rgba(33,150,243,0.15)",   # bleu
                "rgba(76,175,80,0.15)",    # vert
                "rgba(255,152,0,0.15)",    # orange
                "rgba(156,39,176,0.15)",   # violet
                "rgba(0,150,136,0.15)",    # bleu-vert
                "rgba(233,30,99,0.15)",    # rose
                "rgba(121,85,72,0.18)"]    # brun


def fond_cartes(texte, couleur):
    import re
    # Ajoute card_mod (fond) sous chaque « - type: entities » / « - type: markdown » de la section
    return re.sub(r"^( *)- type: (entities|markdown)$",
                  lambda m: (f"{m.group(0)}\n{m.group(1)}  card_mod:\n"
                             f"{m.group(1)}    style: \"ha-card {{ background: {couleur} !important; }}\""),
                  texte, flags=re.M)


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
    for rang, n in enumerate(CAPTEURS):
        if not n.get("vue", True):
            continue
        debut = len(o)
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
                format: relative
              - entity: sensor.{i}_version
                name: Firmware
              - type: attribute
                entity: sensor.{i}_version
                attribute: compilation
                name: Compilé le""")
        if n["mesures"]:
            w("""          - type: entities
            title: Mesures
            entities:""")
            for e, lib in n["mesures"]:
                w(f"""              - entity: {e}
                name: {lib}""")
        if n.get("jn"):
            w("""          - type: entities
            title: Jour / nuit (réglages reçus ; SET jn_... par la commande libre)
            entities:""")
            for k, lib, unite in PARAMS_JN:
                w(f"""              - type: attribute
                entity: sensor.{i}_config
                attribute: {k}
                name: {lib}""")
                if unite:
                    w(f'                suffix: " {unite}"')
            # Rappel de la syntaxe, sous les valeurs (détail dans l'aide en bas de la vue)
            w("""          - type: markdown
            content: |
              Ex. : `SET jn_sbas 15 jn_shaut 25` (commande libre) — clés : `jn_sbas` `jn_shaut` (0 à 100 %, bas < haut),
              `jn_ecl` 0/1, `jn_inter` (1 à 3600 s), `jn_force` (60 à 86400 s)""")
        if n.get("fl"):
            w("""          - type: entities
            title: Flotteur (réglages reçus ; SET mfv_... par la commande libre)
            entities:""")
            for k, lib, unite in PARAMS_FL:
                w(f"""              - type: attribute
                entity: sensor.{i}_config
                attribute: {k}
                name: {lib}""")
                if unite:
                    w(f'                suffix: " {unite}"')
            w("""          - type: markdown
            content: |
              Ex. : `SET mfv_revmin 300 mfv_ack 1` (commande libre) — clés : `mfv_rev` 0/1 (réveil par le flotteur),
              `mfv_revmin` (0 à 86400 s), `mfv_ack` 0/1 (confirmation par le contrôleur), `mfv_ackn` (1 à 10 envois)""")
        w(f"""          - type: markdown
            content: |
              {{% if state_attr('sensor.{i}_config', 'mute') == 1 %}}
              🔇 **Mesures muettes** : {nom} n'émet plus ses mesures (MUTE OFF pour les rétablir).

              {{% endif %}}""")
        if wifi and n.get("ip"):
            w(f"""              {{% set ip = states('{n['ip']}') %}}
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
        w(f"""              - entity: input_boolean.{i}_mute
                name: Mesures muettes""")
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
        o[debut:] = [fond_cartes("\n".join(o[debut:]), FONDS_NOEUDS[rang % len(FONDS_NOEUDS)])]
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
          | `SET cle:valeur ...` | Modifie des paramètres : `sleepA` 0/1, `sleepI` `sleepS` `sleepM` `sleepL` (1 à 86400), `wakeI` (1 à 3600), `mute` 0/1 |
          | `SET jn_sbas 15 jn_shaut 25` | Détecteur jour / nuit (ThCave) : `jn_sbas` seuil bas jour → nuit, `jn_shaut` seuil haut nuit → jour (0 à 100 %, bas < haut), `jn_ecl` remonter l'éclairement 0/1, `jn_inter` intervalle de mesure (1 à 3600 s), `jn_force` délai max sans envoi (60 à 86400 s) |
          | `SET mfv_revmin 300 mfv_ack 1` | Flotteur (ThRemise, Capteur 3.1) : `mfv_rev` réveil par le flotteur 0/1, `mfv_revmin` anti-rafale (0 à 86400 s), `mfv_ack` confirmation par le contrôleur 0/1, `mfv_ackn` nombre d'envois sans confirmation avant de dormir (1 à 10) |
          | `MUTE ON` / `MUTE OFF` | Mesures muettes : plus aucune mesure émise (réponses aux commandes, REVEIL, DEEPSLEEP ON et ALIVE inchangés), enregistré dans le nœud ; en LoRa, une trame « MUET » remplace les mesures (le relais peut ainsi encore lui envoyer ses commandes) |
          | `SLEEP ENABLE` / `SLEEP DISABLE` | Active / désactive le deep sleep |
          | `SLEEP n` | Intervalle = n s, et active le deep sleep |
          | `SLEEPS` / `SLEEPM` / `SLEEPL` | Applique la durée courte / moyenne / longue |
          | `SLEEPS n` (idem M, L) | Règle la durée courte (moyenne, longue) sans l'appliquer |
          | `WDOG ENABLE` / `WDOG DISABLE` / `WDOG s` | Watchdog ALIVE (ignoré en deep sleep) |
          | `STATUS` | Rapport d'état (bouton État) |
          | `REBOOT` | Redémarre le nœud |
          | `ANNULER` | Vide la commande en attente (LoRa : traitée par le relais ; Wi-Fi : efface le message retenu) |

          **Après un flash** : cliquer d'abord sur « Relire », pour que HA connaisse la config du nouveau firmware
          (sinon Enregistrer ignore les réglages que l'ancienne config ne contenait pas, comme « Mesures muettes »).

          **Nettoyer retenu** : efface le message retenu du topic `home/conf<nœud>/command` (commande restée en
          retenu, ancien testament « offline »…). Sans effet sur la file du relais.

          L'automatisation « Cuve - SLEEP et ACK flotteur » n'envoie pas son SLEEP quand une commande attend.

          Page du relais (réseau local) : [Capteurs LoRa](http://{{ states('input_text.ip_relaislora') }}/capteurs)
""")
    return "\n".join(o)


# ============================================================ CARTE D'UN ACTIONNEUR
# Carte (vertical-stack) à coller dans la vue de l'actionneur : complète ce qui y est déjà (présence, RSSI, bouton)
def carte_actionneur(n):
    i, nom, titre = n["id"], n["nom"], n.get("titre", n["nom"])
    o = []
    w = o.append
    w(f"""# Carte « {titre} » pour le tableau Accueil → vue Actionneurs (https://ha.jbil.org/dashboard-accueil/lumieres)
# Généré par genere_noeuds.py (entités et scripts : packages/noeuds_capteurs.yaml).
# Installation : vue Actionneurs → ✏️ → « + Ajouter une carte » dans la section du chauffage → « Manuel » → coller → Enregistrer.
type: vertical-stack
cards:
  # Titre porté par la carte entities : une carte heading en tête de pile était recouverte par la suivante
  - type: entities
    title: {titre}
    icon: mdi:remote
    entities:
      - entity: sensor.{i}_derniere_trame
        name: Dernière trame
        format: relative
      - entity: sensor.{i}_commande_en_attente
        name: Commande en attente
      - entity: sensor.{i}_version
        name: Firmware
      - type: attribute
        entity: sensor.{i}_version
        attribute: compilation
        name: Compilé le
  - type: markdown
    content: |
      {{% set ip = states('{n['ip']}') %}}
      {{% if ip not in ['unknown', 'unavailable', ''] %}}
      **{nom}** : [page Web](http://{{{{ ip }}}}/) · [journal](http://{{{{ ip }}}}/logs) ({{{{ ip }}}})

      {{% endif %}}
      {{% set r = states('sensor.{i}_rapport') %}}
      {{% if r not in ['unknown', 'unavailable'] %}}
      **État** (reçu {{{{ relative_time(as_datetime(state_attr('sensor.{i}_rapport', 'recu'))) }}}}) :

      {{{{ r | replace(' | ', '  \\n') }}}}
      {{% else %}}
      *Pas encore de réponse au bouton État (le nœud n'est joignable que chauffage en marche).*
      {{% endif %}}
  - type: horizontal-stack
    cards:""")
    w(bouton("État", "mdi:information", "noeud_commande", i, "STATUS", indent=6))
    w(bouton("Annuler", "mdi:cancel", "noeud_commande", i, "ANNULER", indent=6))
    w(bouton("Redémarrer", "mdi:restart", "noeud_commande", i, "REBOOT",
             confirmation=f"Redémarrer le {titre} (seulement s'il est sous tension) ?", rouge=True, indent=6))
    w(f"""  - type: entities
    entities:
      - entity: input_text.{i}_commande
        name: Commande libre
  - type: horizontal-stack
    cards:""")
    w(bouton("Envoyer", "mdi:send", "noeud_commande_libre", i, indent=6))
    w(bouton("Nettoyer retenu", "mdi:broom", "noeud_nettoyer_retenu", i,
             confirmation=f"Effacer le message retenu sur home/conf{i}/command ?", indent=6))
    w("")
    return "\n".join(o)


# ============================================================ VUE ACCUEIL → ACTIONNEURS
# Vue complète = celle de l'export .storage/lovelace.dashboard_accueil (à rafraîchir depuis HA avant de régénérer si la
# vue a été modifiée dans HA) + la carte de chaque actionneur, placée juste après la pile des boutons (remplacée si présente).
EXPORT_ACCUEIL = os.path.join(HA, ".storage", "lovelace.dashboard_accueil")
VUE_ACTIONNEURS = "lumieres"
# Bouton de la lumière de la cave (switch.lumiere_cave / switch.lumiere_cave_enable de configuration.yaml),
# même présentation que le projecteur : cadenas (ENABLE / DISABLE) + bouton (ON / OFF)
LUMIERE_CAVE_ENTITE = "switch.lumiere_cave"
CARTE_LUMIERE_CAVE = """
type: horizontal-stack
cards:
  - type: custom:button-card
    entity: switch.lumiere_cave_enable
    name: ''
    show_name: false
    tap_action:
      action: toggle
    icon: |
      [[[return states['switch.lumiere_cave_enable'].state === 'on' ? 'mdi:lock-open' : 'mdi:lock']]]
    styles:
      card:
        - height: 60px
        - width: 60px
        - background: |
            [[[return states['switch.lumiere_cave_enable'].state === 'on' ? 'rgba(0,200,0,0.2)' : 'rgba(200,0,0,0.2)']]]
      icon:
        - color: |
            [[[return states['switch.lumiere_cave_enable'].state === 'on' ? 'green' : 'red']]]
  - type: custom:button-card
    entity: switch.lumiere_cave
    name: Lumière cave
    tap_action:
      action: toggle
    icon: mdi:lightbulb
    styles:
      card:
        - height: 60px
        - background: |
            [[[return states['switch.lumiere_cave'].state === 'on' ? 'rgba(0,200,0,0.2)' : 'rgba(200,0,0,0.2)']]]
      icon:
        - color: |
            [[[return states['switch.lumiere_cave'].state === 'on' ? 'green' : 'red']]]
"""
# Indicateur jour / nuit de la cave (sensor.jncave_etat / sensor.jncave_eclairement de configuration.yaml), juste
# au-dessus du bouton de la lumière cave : même présentation que l'indicateur « C3 » au-dessus du chauffage SdB
JOUR_NUIT_CAVE_ENTITE = "sensor.jncave_etat"
CARTE_JOUR_NUIT_CAVE = """
type: custom:button-card
entity: sensor.jncave_etat
show_icon: false
show_name: false
show_state: false
show_label: true
tap_action:
  action: none
hold_action:
  action: more-info
label: |
  [[[
    const e = states['sensor.jncave_eclairement'];
    const v = e ? parseFloat(e.state) : NaN;
    if (entity && (entity.state === 'Jour' || entity.state === 'Nuit'))
      return isNaN(v) ? `Cave · ${entity.state}` : `Cave · ${entity.state} · ${v} %`;
    return 'Cave · —';
  ]]]
styles:
  card:
    - height: 40px
    - background: |
        [[[
          if (entity && entity.state === 'Jour') return 'rgba(255,200,0,0.2)';
          if (entity && entity.state === 'Nuit') return 'rgba(0,0,150,0.25)';
          return 'rgba(128,128,128,0.2)';
        ]]]
  label:
    - font-size: 15px
    - font-weight: 600
    - color: |
        [[[
          if (entity && entity.state === 'Jour') return 'darkorange';
          if (entity && entity.state === 'Nuit') return 'royalblue';
          return 'grey';
        ]]]
"""


def vue_actionneurs():
    import json
    import yaml

    class Dumper(yaml.SafeDumper):
        pass

    # Chaînes sur plusieurs lignes (JavaScript des button-card, markdown) en bloc littéral, comme dans l'éditeur de HA
    Dumper.add_representer(str, lambda d, s: d.represent_scalar(
        "tag:yaml.org,2002:str", s, style="|" if "\n" in s else None))
    cfg = json.load(open(EXPORT_ACCUEIL, encoding="utf-8"))["data"]["config"]
    vue = next(v for v in cfg["views"] if v.get("path") == VUE_ACTIONNEURS)
    cartes = vue["sections"][0]["cards"]
    for n in ACTIONNEURS:
        carte = yaml.safe_load(carte_actionneur(n))
        titre = carte["cards"][0]["title"]
        # Remplace la carte déjà présente (titre porté par sa 1re carte : entities, ou heading dans l'ancienne version)
        cartes[:] = [c for c in cartes if not (c.get("type") == "vertical-stack" and c.get("cards")
                                              and titre in (c["cards"][0].get("title"), c["cards"][0].get("heading")))]
        cartes.insert(1, carte)
    # Lumière cave (prise 433 MHz du CYD) : cadenas + bouton comme le projecteur, tout en bas (peu utilisée)
    if not any(LUMIERE_CAVE_ENTITE in json.dumps(c) for c in cartes):
        cartes.append(yaml.safe_load(CARTE_LUMIERE_CAVE))
    # Jour / nuit de la cave : juste au-dessus de la lumière cave (remplacé si présent)
    cartes[:] = [c for c in cartes if c.get("entity") != JOUR_NUIT_CAVE_ENTITE]
    i = next(i for i, c in enumerate(cartes) if LUMIERE_CAVE_ENTITE in json.dumps(c))
    cartes.insert(i, yaml.safe_load(CARTE_JOUR_NUIT_CAVE))
    entete = (f"# Vue « {vue.get('title')} » du tableau Accueil (https://ha.jbil.org/dashboard-accueil/{VUE_ACTIONNEURS})\n"
              "# Généré par genere_noeuds.py à partir de .storage/lovelace.dashboard_accueil + carte(s) des actionneurs.\n"
              "# Installation : tableau Accueil → ✏️ → vue Actionneurs → ⋮ → « Modifier en YAML » → remplacer par ce fichier.\n")
    return entete + yaml.dump(vue, Dumper=Dumper, allow_unicode=True, sort_keys=False, width=1000)


# ============================================================ VUE SYSTÈME → DIAGNOSTIC
# Vue complète = celle de l'export .storage/lovelace.dashboard_systeme (à rafraîchir depuis HA avant de régénérer si la
# vue a été modifiée dans HA) + section « Versions des firmwares » (remplacée si présente), en tête.
EXPORT_SYSTEME = os.path.join(HA, ".storage", "lovelace.dashboard_systeme")
VUE_DIAGNOSTIC = "diagnostic"
TITRE_VERSIONS = "Versions des firmwares"
# (entité, libellé, famille de firmware) : ⚠️ si la version est plus ancienne que la plus récente de sa famille
EQUIPEMENTS_VERSION = (
    [("sensor.cyd_version", "CYD principal", "Home-Assist"),
     ("sensor.cyd_aux_version", "CYD Aux", "Home-Assist"),
     ("sensor.cyd_aux2_version", "CYD Aux 2", "Home-Assist"),
     ("sensor.relaislora_version", "Relais LoRa", "Relais-Lora")]
    + [(f"sensor.{n['id']}_version", n.get("titre", n["nom"]), "Capteur") for n in NOEUDS if n.get("vue", True)])


def carte_versions():
    lignes = ",\n".join(f"    ('{e}', '{lib}', '{fam}')" for e, lib, fam in EQUIPEMENTS_VERSION)
    contenu = ("{%- set eq = [\n" + lignes + "\n] -%}\n"
               "{%- set ns = namespace(max={}) -%}\n"
               "{%- for e, lib, fam in eq -%}\n"
               "  {%- set v = states(e) -%}\n"
               "  {%- if v | regex_match('^[0-9]+(\\.[0-9]+)*$') -%}\n"
               "    {%- set t = v.split('.') | map('int') | list -%}\n"
               "    {%- if fam not in ns.max or t > ns.max[fam] -%}{%- set ns.max = dict(ns.max, **{fam: t}) -%}{%- endif -%}\n"
               "  {%- endif -%}\n"
               "{%- endfor -%}\n"
               "| Équipement | Firmware | Version | Compilée le | Reçue |\n"
               "|:--|:--|:--:|:--:|:--|\n"
               "{%- for e, lib, fam in eq %}\n"
               "{%- set v = states(e) %}\n"
               "{%- set connue = v | regex_match('^[0-9]+(\\.[0-9]+)*$') %}\n"
               "{%- set r = state_attr(e, 'recu') %}\n"
               "| {{ lib }} | {{ fam }} | "
               "{% if not connue %}—{% elif (v.split('.') | map('int') | list) < ns.max[fam] %}⚠️ **{{ v }}**{% else %}{{ v }}{% endif %} | "
               "{{ state_attr(e, 'compilation') or '—' }} | "
               "{{ relative_time(as_datetime(r)) if r else 'jamais' }} |\n"
               "{%- endfor %}\n\n"
               "⚠️ : plus ancienne que la version la plus récente du même firmware. "
               "Envoyée au démarrage, et en réponse à GET (nœuds) ou STATUS (CYD, relais).\n")
    return {"type": "grid", "cards": [
        {"type": "heading", "heading": TITRE_VERSIONS, "heading_style": "title", "icon": "mdi:chip"},
        {"type": "markdown", "content": contenu, "grid_options": {"columns": "full"}}]}


def vue_diagnostic():
    import json
    import yaml

    class Dumper(yaml.SafeDumper):
        pass

    Dumper.add_representer(str, lambda d, s: d.represent_scalar(
        "tag:yaml.org,2002:str", s, style="|" if "\n" in s else None))
    cfg = json.load(open(EXPORT_SYSTEME, encoding="utf-8"))["data"]["config"]
    vue = next(v for v in cfg["views"] if v.get("path") == VUE_DIAGNOSTIC)
    vue.pop("cards", None)  # « cards: [] » ajouté par HA dans une vue en sections
    vue["sections"] = [s for s in vue["sections"]
                       if not (s.get("cards") and s["cards"][0].get("heading") == TITRE_VERSIONS)]
    vue["sections"].insert(0, carte_versions())
    entete = (f"# Vue « {vue.get('title')} » du tableau Système (https://ha.jbil.org/dashboard-systeme/{VUE_DIAGNOSTIC})\n"
              "# Généré par genere_noeuds.py à partir de .storage/lovelace.dashboard_systeme + section « Versions des firmwares »\n"
              "# (sensor.<id>_version : packages/noeuds_capteurs.yaml et packages/cyd.yaml).\n"
              "# Installation : tableau Système → ✏️ → vue Diagnostic → ⋮ → « Modifier en YAML » → remplacer par ce fichier.\n")
    return entete + yaml.dump(vue, Dumper=Dumper, allow_unicode=True, sort_keys=False, width=1000)


# ============================================================ VUE SYSTÈME → RELAIS LORA
def vue_relais():
    noms = ", ".join(f"'{n['id']}': '{n['nom']}'" for n in NOEUDS if n["mode"] == "lora")
    return """# Vue « Relais LoRa » du tableau Système (https://ha.jbil.org/dashboard-systeme/relais-lora)
# Généré par genere_noeuds.py (entités : packages/noeuds_capteurs.yaml ; IP : automatisation « Relais LoRa - Recevoir IP »).
# Installation : tableau Système → ✏️ → « + » (nouvelle vue) → ⋮ → « Modifier en YAML » → remplacer par ce fichier → Enregistrer.
type: sections
max_columns: 4
title: Relais LoRa
path: relais-lora
icon: mdi:router-wireless
show_icon_and_title: true
sections:
  - type: grid
    cards:
      - type: heading
        heading: Relais LoRa
        heading_style: title
        icon: mdi:router-wireless
      - type: markdown
        content: |
          {% set ip = states('input_text.ip_relaislora') %}
          {% if is_state('binary_sensor.relaislora_connexion', 'on') %}🟢 **Connecté**{% elif is_state('binary_sensor.relaislora_connexion', 'off') %}🔴 **Déconnecté**{% else %}⚪ **Inconnu**{% endif %} · IP **{{ ip }}**<br>
          Dernier signal : {% if has_value('sensor.relaislora_dernier_signal') %}il y a {{ relative_time(as_datetime(states('sensor.relaislora_dernier_signal'))) }}{% else %}aucun{% endif %}<br>
          {%- set v = states('sensor.relaislora_version') -%}
          Firmware : {% if v in ['unknown', 'unavailable'] %}*pas encore reçu*{% else %}**{{ v }}**, compilé le {{ state_attr('sensor.relaislora_version', 'compilation') }}{% endif %}

          [🌐 Page Web](http://{{ ip }}/) · [📡 Capteurs LoRa](http://{{ ip }}/capteurs) · [📜 Journal](http://{{ ip }}/logs)
      - type: button
        name: État
        icon: mdi:information
        tap_action:
          action: perform-action
          perform_action: mqtt.publish
          data:
            topic: home/confrelaislora/command
            payload: STATUS
        hold_action:
          action: none
        grid_options:
          columns: 6
          rows: 1
      - type: button
        name: Redémarrer
        icon: mdi:restart
        tap_action:
          action: perform-action
          perform_action: mqtt.publish
          data:
            topic: home/confrelaislora/command
            payload: REBOOT
          confirmation:
            text: Redémarrer le relais LoRa ? Les commandes en attente pour les nœuds LoRa seront perdues.
        hold_action:
          action: none
        grid_options:
          columns: 6
          rows: 1
        card_mod:
          style: |
            ha-card {
              background: rgba(200,0,0,0.2) !important;
            }
            ha-icon, ha-state-icon { color: red !important; }
  - type: grid
    cards:
      - type: heading
        heading: État (réponse au bouton État)
        heading_style: title
        icon: mdi:clipboard-text-outline
      - type: markdown
        content: |
          {%- set t = state_attr('sensor.relaislora_etat', 'texte') -%}
          {%- if not t -%}
          *Pas encore de réponse : bouton État.*
          {%- else -%}
          {%- set noms = {""" + noms + """} -%}
          {%- set lignes = t.split('\\n') -%}
          Reçu il y a {{ relative_time(as_datetime(states('sensor.relaislora_etat'))) }} (horloge du relais : {{ lignes[1] }})

          | | |
          |:--|:--|
          {%- for l in lignes[2:] if ' : ' in l and not l.startswith('home/') %}
          {%- set k = l.split(' : ', 1)[0] | trim %}
          {%- set v = l.split(' : ', 1)[1] | trim %}
          {%- if k == 'Uptime' %}{% set n = v | replace('s', '') | int(0) %}{% set v = (n // 86400) ~ ' j ' ~ ((n % 86400) // 3600) ~ ' h ' ~ ((n % 3600) // 60) ~ ' min' %}{% endif %}
          | {{ k }} | {{ v }} |
          {%- endfor %}

          **Files d'attente des nœuds LoRa**

          | Nœud | Dernière trame | En attente |
          |:--|:--|:--|
          {%- for l in lignes if l.startswith('home/') %}
          {%- set id = l.split(' : ', 1)[0] | replace('home/conf', '') | replace('/command', '') %}
          {%- set p = l.split(' : ', 1)[1].split(' — en attente : ') %}
          | {{ noms.get(id, id) }} | {{ p[0] | replace('dernière trame ', '') }} | {{ p[1] if p | count > 1 else 'aucune' }} |
          {%- endfor %}
          {%- endif %}
"""


def ecrit(chemin, texte):
    with open(chemin, "w", encoding="utf-8", newline="\r\n") as f:
        f.write(texte)
    print("écrit :", chemin)


ecrit(os.path.join(HA, "packages", "noeuds_capteurs.yaml"), paquet())
ecrit(os.path.join(HA, "vue-noeuds-capteurs.yaml"), vue())
ecrit(os.path.join(HA, "vue-actionneurs.yaml"), vue_actionneurs())
ecrit(os.path.join(HA, "vue-diagnostic.yaml"), vue_diagnostic())
ecrit(os.path.join(HA, "vue-relais-lora.yaml"), vue_relais())
