#FIXME: gpgme++ is in kdepimlibs, must move somewhere else!
find_package(KdepimLibs 4.5.60)
set_package_properties(KDEPIMLIBS PROPERTIES DESCRIPTION "KDE PIM libraries"
                       URL "http://www.kde.org" TYPE OPTIONAL
                       PURPOSE "Needed for building several Plasma DataEngines")
include_directories(${KDEPIMLIBS_INCLUDE_DIRS})

