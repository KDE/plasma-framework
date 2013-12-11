project("plasmapkg")

add_executable(plasmapkg
    main.cpp
    plasmapkg.cpp
)

target_link_libraries(plasmapkg KF5::Plasma KF5::I18n KF5::Service KF5::CoreAddons Qt5::DBus)

message("INSTALL_TARGETS_DEFAULT_ARGS ${INSTALL_TARGETS_DEFAULT_ARGS}")

install(TARGETS plasmapkg ${INSTALL_TARGETS_DEFAULT_ARGS})

