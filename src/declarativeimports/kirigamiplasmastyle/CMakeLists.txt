
add_library(KirigamiPlasmaStyle MODULE ${org.kde.desktop_SRCS})

target_sources(KirigamiPlasmaStyle PRIVATE
    plugin.cpp
    units.cpp
    plasmatheme.cpp
)

target_link_libraries(KirigamiPlasmaStyle
    PUBLIC
        Qt6::Core
        KF6::Kirigami2
    PRIVATE
        Qt6::Qml
        Qt6::Quick
        KF6::ConfigWidgets
        KF6::IconThemes
        KF6::Plasma
)

install(TARGETS KirigamiPlasmaStyle DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/kirigami)

install(FILES
    AbstractApplicationHeader.qml
    Icon.qml
    Theme.qml
    DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/kirigami/styles/Plasma
)
