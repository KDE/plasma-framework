add_library(plasmaextracomponentsplugin SHARED)

target_sources(plasmaextracomponentsplugin PRIVATE
    plasmaextracomponentsplugin.cpp
    enums.cpp
    qmenu.cpp
    qmenuitem.cpp
)

target_link_libraries(plasmaextracomponentsplugin
        Qt6::Quick
        Qt6::Qml
        Qt6::Widgets
        KF6::ConfigWidgets
        KF6::Plasma)


install(TARGETS plasmaextracomponentsplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/extras)

install(DIRECTORY qml/ DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/extras)

ecm_generate_qmltypes(org.kde.plasma.extras 2.0 DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/extras)
