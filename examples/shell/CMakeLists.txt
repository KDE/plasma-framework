add_executable(exampleplasmashell)

target_sources(exampleplasmashell PRIVATE
    customcorona.cpp
    main.cpp
)

target_link_libraries(exampleplasmashell
    Qt6::Widgets
    Qt6::Quick
    Qt6::Qml
    KF6::I18n
    Plasma::PlasmaQuick
    Plasma::Plasma
    KF6::DBusAddons
    KF6::Notifications
    KF6::Package
)

