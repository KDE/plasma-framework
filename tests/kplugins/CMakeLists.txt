add_executable(plugintest
    main.cpp
    plugintest.cpp
)

target_link_libraries(plugintest KF5::Plasma KF5::I18n Qt5::DBus Qt5::Widgets)

#install(TARGETS plugintest ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

