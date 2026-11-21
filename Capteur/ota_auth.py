# -*- coding: utf-8 -*-
# Script PlatformIO (extra_scripts = post:ota_auth.py) : téléversement par le réseau avec espota.py (outil du framework
# Arduino) dès que le port de téléversement est une adresse IP (upload_port du profil, ou --upload-port <IP>) ; sinon
# rien ne change (USB, esptool). Mot de passe lu dans src/secret.h (OTA_MOT_DE_PASSE) : secret.h n'est pas commité, le
# mot de passe ne figure ainsi ni dans platformio.ini ni dans le dépôt. Copie identique dans Relais-Lora, Home-Assist
# et Capteur.
#
# Pas de upload_protocol dans les profils : dès qu'il y en a un (espota, custom...), la plateforme pioarduino (relais)
# installe openocd depuis GitHub (inutile ici, et en échec : certificat refusé, 21/11/2026). Script « post: » : remplace
# la commande esptool, définie par la plateforme, par espota.
#   Ex. : pio run -e ThChRDC_Xiao_C3 -t upload --upload-port 192.168.1.128
#         pio run -e ThCave_Xiao_S3 -t upload --upload-port 192.168.4.1   (point d'accès « ThCave-MAJ », commande OTA)
import os
import re
Import("env")

port = env.subst("$UPLOAD_PORT")
if re.match(r"^\d{1,3}(\.\d{1,3}){3}$", port or ""):
    chemin = os.path.join(env.subst("$PROJECT_DIR"), "src", "secret.h")
    mdp = ""
    if os.path.exists(chemin):
        with open(chemin, encoding="utf-8") as f:
            m = re.search(r'#define\s+OTA_MOT_DE_PASSE\s+"([^"]*)"', f.read())
            if m:
                mdp = m.group(1)
    if not mdp:
        print("ota_auth.py : OTA_MOT_DE_PASSE introuvable dans src/secret.h, téléversement réseau sans mot de passe")
    espota = os.path.join(env.PioPlatform().get_package_dir("framework-arduinoespressif32"), "tools", "espota.py")
    cmd = '"$PYTHONEXE" "%s" -i $UPLOAD_PORT -p 3232 -f "$SOURCE"' % espota
    if mdp:
        cmd += ' -a "%s"' % mdp
    env.Replace(UPLOADCMD=cmd)
    print("ota_auth.py : port %s = adresse IP, téléversement par le réseau (espota)" % port)
