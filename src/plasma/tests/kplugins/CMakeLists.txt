project("plugintest")

add_executable(plugintest
    main.cpp
    plugintest.cpp
)

target_link_libraries(plugintest plasma ${KI18n_LIBRARIES} KF5::KService Qt5::DBus)

message("INSTALL_TARGETS_DEFAULT_ARGS ${INSTALL_TARGETS_DEFAULT_ARGS}")

#install(TARGETS plugintest ${INSTALL_TARGETS_DEFAULT_ARGS})

