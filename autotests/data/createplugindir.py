#!/usr/bin/python -u
__doc__ = """

This script creates a directory filled with fake Plasma packages

The packages are empty skeletons just containing a metadata file
and an empty contents subdirectory. The packages are numbered.
The script creates pnum packages

"""

import os

debug = False # actually do something
pnum = 100 # number of packages per servicetype to create

datadirs = os.popen("kf5-config --path data").readlines()[0][:-1].split(":")
cwd = os.popen("pwd").readlines()[0][:-1]

pluginDirs = {}


pluginDirs["Plasma/Applet"] = "plasmoids"
pluginDirs["Plasma/Containment"] = "plasmoids"
pluginDirs["Plasma/LayoutTemplate"] = "layout-templates"
pluginDirs["Plasma/LookAndFeel"] = "look-and-feel"
pluginDirs["Plasma/Generic"] = "packages"
pluginDirs["Plasma/Shell"] = "shells"
pluginDirs["Plasma/Wallpaper"] = "wallpapers"


# Read template
template = ""

for l in open("template-metadata.desktop", "r").readlines():
    template = template + l

#print template


def writePackage(servicetype, index):
    pluginname = "org.kde.test."+pluginDirs[servicetype].replace("-", "")+index
    packagepath = cwd+"/plasma/"+pluginDirs[servicetype]+"/"+pluginname+"/"


    output = template.replace("NUMBER", N)
    output = output.replace("SERVICETYPE", servicetype)
    output = output.replace("PLUGINNAME", pluginname)

    if not debug:
        try: os.makedirs(packagepath+"contents ")
        except: print(""), # no worries

        metadata = open(packagepath+"metadata.desktop", "w")
        metadata.write(output)
        metadata.close()
    #print("Wrote package: " + packagepath)

if not debug:
    print("recreating " + cwd+"/plasma from scratch...")
    os.system("rm -rf "+cwd+"/plasma")

for servicetype in pluginDirs.keys():
    for n in range(pnum):
        N = str(n)
        writePackage(servicetype, N)

if not debug:
    #os.system("zip -r plasmaplugins.zip plasma")
    #os.system("rm -rf plasma")
    print("")

print("Wrote " + str(pnum*len(pluginDirs.keys())) + " fake packages for " + str(len(pluginDirs.keys())) + " servicetypes." );