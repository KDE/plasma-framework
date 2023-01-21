add_executable(exampleplasmashell)

target_sources(exampleplasmashell PRIVATE
    customcorona.cpp
    main.cpp
)

target_link_libraries(exampleplasmashell
    Qt${QT_MAJOR_VERSION}::Widgets
    Qt${QT_MAJOR_VERSION}::Quick
    Qt${QT_MAJOR_VERSION}::Qml
    KF6::I18n
    KF6::XmlGui
    KF6::PlasmaQuick
    KF6::Plasma
    KF6::DBusAddons
    KF6::Notifications
    KF6::QuickAddons
    KF6::Package
)

