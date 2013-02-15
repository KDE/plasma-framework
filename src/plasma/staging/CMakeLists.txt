#FIXME: gpgme++ is in kdepimlibs, must move somewhere else!
macro_optional_find_package(KdepimLibs 4.5.60)
macro_log_feature(KDEPIMLIBS_FOUND "kdepimlibs" "KDE PIM libraries" "http://www.kde.org" FALSE "" "Needed for building several Plasma DataEngines")
include_directories(${KDEPIMLIBS_INCLUDE_DIRS})

