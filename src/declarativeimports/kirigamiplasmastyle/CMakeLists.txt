
add_library(KirigamiPlasmaStyle MODULE ${org.kde.desktop_SRCS})

target_sources(KirigamiPlasmaStyle PRIVATE
    plugin.cpp
    units.cpp
)

target_link_libraries(KirigamiPlasmaStyle
    PUBLIC
        Qt${QT_MAJOR_VERSION}::Core
        KF5::Kirigami2
    PRIVATE
        Qt${QT_MAJOR_VERSION}::DBus
        Qt${QT_MAJOR_VERSION}::Qml
        Qt${QT_MAJOR_VERSION}::Quick
        KF5::ConfigWidgets
        KF5::IconThemes
)

install(TARGETS KirigamiPlasmaStyle DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf5/kirigami)

install(FILES
    AbstractApplicationHeader.qml
    Icon.qml
    Theme.qml
    DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/kirigami.2/styles/Plasma
)
