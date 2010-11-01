#! /usr/bin/env bash
$EXTRACTRC private/*.ui >> rc.cpp
$XGETTEXT `ls *.cpp *.h */*.h */*.cpp | grep -v 'tests/'` -o $podir/libplasma.pot
rm -f *.ui
