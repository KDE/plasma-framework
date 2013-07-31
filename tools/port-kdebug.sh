#!/bin/sh

# kDebug() becomes qDebug() in cpp files

for FS in `find $PWD -type f -name '*.cpp'`; do
    perl -p -i -e 's/kDebug\(\)/qDebug()/g' $FS
done

#exit;

