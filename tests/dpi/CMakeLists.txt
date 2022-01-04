add_executable(dpitest
    main.cpp
    dpitest.cpp
)

target_link_libraries(dpitest KF5::Plasma KF5::I18n Qt${QT_MAJOR_VERSION}::Gui)

