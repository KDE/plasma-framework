project("plasmapkg")

add_executable(plasmapkg
    main.cpp
    plasmapkg.cpp
)

target_link_libraries(plasmapkg plasma ${KI18n_LIBRARIES} KF5::KService KF5::KCoreAddons Qt5::DBus)

message("INSTALL_TARGETS_DEFAULT_ARGS ${INSTALL_TARGETS_DEFAULT_ARGS}")

install(TARGETS plasmapkg ${INSTALL_TARGETS_DEFAULT_ARGS})

