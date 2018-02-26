#! /usr/bin/env bash
$XGETTEXT `find . -name \*.qml -o -name \*.cpp` -o $podir/plasma_wallpaper_org.kde.plasma.%{APPNAMELC}.pot
