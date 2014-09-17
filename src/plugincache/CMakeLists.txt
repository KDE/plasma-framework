project("plasma-update-packagecache")

include(ECMMarkNonGuiExecutable)

add_executable(plasma-update-packagecache
    main.cpp
    kconfigtojson.cpp
)
add_executable(KF5::plasma-update-packagecache ALIAS plasma-update-packagecache)

# Mark it as non-gui so we won't create an app bundle on Mac OS X
ecm_mark_nongui_executable(plasma-update-packagecache)

target_link_libraries(plasma-update-packagecache
    Qt5::Core
    KF5::ConfigCore
)

install(TARGETS plasma-update-packagecache EXPORT KF5PlasmaTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})
