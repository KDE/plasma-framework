#!/bin/sh

#If this test fails it means you are probably using i18n() in your QML code
#This should be replaced by i18nd in order to for i18n to load the correct catalog

#First arg should be the directory to check

! find "$1" -name '*.qml' -print0 | xargs -0 grep 'i18n[^d]*('

