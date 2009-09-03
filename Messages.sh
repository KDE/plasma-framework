#! /usr/bin/env bash
$EXTRACTRC private/*.ui >> rc.cpp
$XGETTEXT *.cpp *.h */*.h */*.cpp -o $podir/libplasma.pot
rm -f *.ui
