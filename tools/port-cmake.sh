#!/bin/sh
#    ${KDE4_KIDLETIME_LIBRARY}
#    ${KIdleTime_LIBRARIES}
#     ${KCore_LIBRARIES}
#     ${Solid_LIBRARIES}
#     ${Plasma_LIBRARIES}
# ./geolocation/CMakeLists.txt:29:    ${KDE4_PLASMA_LIBS}
# ./geolocation/CMakeLists.txt:30:    ${KDE4_KDECORE_LIBS}
# ./geolocation/CMakeLists.txt:31:    ${KDE4_KIO_LIBS}
# ./geolocation/CMakeLists.txt:32:    ${KDE4_SOLID_LIBS})


for FS in `find $PWD -type f -name 'CMakeLists.txt'`; do
    perl -p -i -e 's/kde4_add_plugin/plasma_add_plugin/g' $FS

    perl -p -i -e 's/KDE4_KIDLETIME_LIBRARY/KIdleTime_LIBRARIES/g' $FS
    perl -p -i -e 's/KDE4_PLASMA_LIBS/Plasma_LIBRARIES/g' $FS
    perl -p -i -e 's/\$\{KDE4_KDECORE_LIBS\}//g' $FS
    perl -p -i -e 's/KDE4_SOLID_LIBS/KF5\:\:Solid/g' $FS
    perl -p -i -e 's/\$\{Solid_LIBRARIES\}/KF5\:\:Solid/g' $FS
    perl -p -i -e 's/\$\{KService_LIBRARIES\}/KF5\:\:KService/g' $FS
    perl -p -i -e 's/\$\{KCoreAddons_LIBRARIES\}/KF5\:\:KCoreAddons/g' $FS
    perl -p -i -e 's/\$\{KI18n_LIBRARIES\}/KF5\:\:KI18n/g' $FS
    perl -p -i -e 's/\$\{KArchive_LIBRARIES\}/KF5\:\:KArchive/g' $FS
done

