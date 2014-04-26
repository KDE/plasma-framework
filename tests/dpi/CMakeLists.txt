add_executable(dpitest
    main.cpp
    dpitest.cpp
)

target_link_libraries(dpitest KF5::Plasma KF5::I18n Qt5::Gui)

message("INSTALL_TARGETS_DEFAULT_ARGS ${INSTALL_TARGETS_DEFAULT_ARGS}")

install(TARGETS dpitest ${INSTALL_TARGETS_DEFAULT_ARGS})

