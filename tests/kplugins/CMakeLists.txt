add_executable(plugintest
    main.cpp
    plugintest.cpp
)

target_link_libraries(plugintest KF6::Plasma KF6::I18n Qt6::DBus Qt6::Widgets)

#install(TARGETS plugintest ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

