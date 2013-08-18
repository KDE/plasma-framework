#!/bin/sh

# slots -> Q_SLOTS, signals -> Q_SIGNALS

for FS in `find $PWD -type f -name '*.h'`; do
    perl -p -i -e 's/ slots\:/ Q_SLOTS\:/g' $FS
    perl -p -i -e 's/signals\:/Q_SIGNALS\:/g' $FS
done

#exit;

