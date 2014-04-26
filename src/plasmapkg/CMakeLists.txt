add_executable(plasmapkg2
    main.cpp
    plasmapkg.cpp
)

target_link_libraries(plasmapkg2 KF5::Plasma KF5::I18n KF5::Service KF5::CoreAddons Qt5::DBus)

message("INSTALL_TARGETS_DEFAULT_ARGS ${INSTALL_TARGETS_DEFAULT_ARGS}")

install(TARGETS plasmapkg2 ${INSTALL_TARGETS_DEFAULT_ARGS})

