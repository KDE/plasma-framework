project("plugintest")

add_executable(plugintest
    main.cpp
    plugintest.cpp
)

target_link_libraries(plugintest KF5::Plasma KF5::I18n KF5::Service Qt5::DBus)

message("INSTALL_TARGETS_DEFAULT_ARGS ${INSTALL_TARGETS_DEFAULT_ARGS}")

#install(TARGETS plugintest ${INSTALL_TARGETS_DEFAULT_ARGS})

