#!/bin/sh

# kDebug() becomes qDebug() in cpp files

for FS in `find $PWD  -name '*.h' -o -name '*.cpp'`; do
    perl -p -i -e 's/\#include \<KDebug\>/\#include \<QDebug\>/g' $FS
    perl -p -i -e 's/\#include \<KDebug\>/\#include \<QDebug\>/g' $FS
    perl -p -i -e 's/\#include \<kdebug\.h\>/\#include \<QDebug\>/g' $FS
    perl -p -i -e 's/kDebug\(\)/qDebug()/g' $FS
    perl -p -i -e 's/kWarning\(\)/qWarning()/g' $FS

    perl -p -i -e 's/kDebug\(250\)/\/\/qDebug()/g' $FS
    perl -p -i -e 's/kDebug\(96669\)/\/\/qDebug()/g' $FS
    perl -p -i -e 's/kDebug\(kdbg_code\)/\/\/qDebug()/g' $FS
done

