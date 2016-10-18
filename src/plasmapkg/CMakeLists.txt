add_executable(plasmapkg2
    main.cpp
    plasmapkg.cpp
)

target_link_libraries(plasmapkg2 KF5::Plasma KF5::I18n KF5::Service KF5::CoreAddons Qt5::DBus)

install(TARGETS plasmapkg2 ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

