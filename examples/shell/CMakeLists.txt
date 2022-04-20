add_executable(exampleplasmashell)

target_sources(exampleplasmashell PRIVATE
    customcorona.cpp
    main.cpp
)

target_link_libraries(exampleplasmashell
    Qt${QT_MAJOR_VERSION}::Widgets
    Qt${QT_MAJOR_VERSION}::Quick
    Qt${QT_MAJOR_VERSION}::Qml
    KF5::I18n
    KF5::XmlGui
    KF5::PlasmaQuick
    KF5::Plasma
    KF5::DBusAddons
    KF5::Notifications
    KF5::QuickAddons
    KF5::Package
)

