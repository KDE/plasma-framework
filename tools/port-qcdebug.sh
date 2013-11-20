#!/bin/sh

# Port all qDebug() statements to qCDebug()
# change all QDebug includes to QLoggingCategory

# Note: don't forget to put the right debug area here,
# i.e. replace SYSTEMTRAY with whatever you put into
# the Q_LOGGING_CATEGORY(SYSTEMTRAY, "systemtray") macro


for FS in `find $PWD  -name '*.h' -o -name '*.cpp'`; do
    perl -p -i -e 's/qDebug\(\)/qCDebug(SYSTEMTRAY)/g' $FS
    perl -p -i -e 's/\#include \<QDebug\>/\#include \<QLoggingCategory\>/g' $FS
done
