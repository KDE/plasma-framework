#!/bin/sh
# it will remove the module prefix in Qt includes, it's usually a problem
# since many classes have changed module between Qt4 and Qt5

for FS in `find $PWD -type f -name '*.h' -or -name '*.cpp'`; do
    perl -p -i -e 's/#include\s+<Qt\w+\/(\w*)>/#include <\1>/g' $FS
    perl -p -i -e 's/#include\s+<KDE\/(\w*)>/#include <\1>/g' $FS
done
