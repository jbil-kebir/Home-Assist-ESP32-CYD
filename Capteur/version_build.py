# -*- coding: utf-8 -*-
# Script PlatformIO (extra_scripts = pre:version_build.py) : supprime l'objet de MyVersion.cpp avant chaque
# compilation, pour que __DATE__ / __TIME__ (date et heure de compilation, voir MyVersion.h) soient toujours à jour.
import os
Import("env")

objet = os.path.join(env.subst("$BUILD_DIR"), "src", "MyVersion.cpp.o")
if os.path.exists(objet):
    os.remove(objet)
