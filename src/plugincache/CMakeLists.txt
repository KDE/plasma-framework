project("desktoptojson")

include(ECMMarkNonGuiExecutable)

add_executable(desktoptojson
    main.cpp
    kconfigtojson.cpp
)
add_executable(KF5::desktoptojson ALIAS desktoptojson)

# Mark it as non-gui so we won't create an app bundle on Mac OS X
ecm_mark_nongui_executable(desktoptojson)

target_link_libraries(desktoptojson
    Qt5::Core
    KF5::ConfigCore
)

install(TARGETS desktoptojson EXPORT KF5ServiceTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})
