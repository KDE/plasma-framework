#find_package(KF5Declarative REQUIRED NO_MODULE)

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
        KF5::WidgetsAddons # for KAcceleratorManager
        KF5::Declarative
        KF5::Plasma
        KF5::IconThemes
        KF5::Service #for kplugininfo.h
)


install(TARGETS plasmacomponentsplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/components)

install(DIRECTORY qml/ DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/components)

ecm_generate_qmltypes(org.kde.plasma.components 2.0 DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/components)
