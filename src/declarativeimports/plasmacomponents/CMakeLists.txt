#find_package(KF6Declarative REQUIRED NO_MODULE)

add_library(plasmacomponentsplugin SHARED)

target_sources(plasmacomponentsplugin PRIVATE
    #fullscreensheet.cpp
    #fullscreenwindow.cpp
    plasmacomponentsplugin.cpp
    qrangemodel.cpp
    enums.cpp
    qmenu.cpp
    qmenuitem.cpp
    )

target_link_libraries(plasmacomponentsplugin
        Qt${QT_MAJOR_VERSION}::Core
        Qt${QT_MAJOR_VERSION}::Quick
        Qt${QT_MAJOR_VERSION}::Qml
        Qt${QT_MAJOR_VERSION}::Gui
        KF6::WidgetsAddons # for KAcceleratorManager
        KF6::Declarative
        KF6::Plasma
        KF6::IconThemes
        KF6::Service #for kplugininfo.h
)


install(TARGETS plasmacomponentsplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/components)

install(DIRECTORY qml/ DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/components)

ecm_generate_qmltypes(org.kde.plasma.components 2.0 DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/components)
