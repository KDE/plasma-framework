# APPLET

if(KDE_PLATFORM_FEATURE_BINARY_COMPATIBLE_FEATURE_REDUCTION)
    set(PLASMA_NO_KDEWEBKIT TRUE)
    set(PLASMA_NO_SOLID TRUE)
endif()

# PLASMOID LIBRARY

add_library(plasma_appletscript_object SHARED)

target_sources(plasma_appletscript_object PRIVATE
    plasmoid/dropmenu.cpp
    plasmoid/appletinterface.cpp
    plasmoid/containmentinterface.cpp
    plasmoid/declarativeappletscript.cpp
    plasmoid/wallpaperinterface.cpp
)

target_link_libraries(plasma_appletscript_object PUBLIC
    Qt${QT_MAJOR_VERSION}::Quick
    Qt${QT_MAJOR_VERSION}::Qml
    KF5::Activities
    KF5::ConfigQml
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::Declarative
    KF5::I18n
    KF5::XmlGui # KActionCollection
    KF5::Plasma
    KF5::PlasmaQuick
    KF5::Package
    KF5::Notifications
)

# DECLARATIVE APPLET
add_library(plasma_appletscript_declarative MODULE)
target_sources(plasma_appletscript_declarative PRIVATE
    plasmoid/declarativeappletscriptplugin.cpp
)

target_link_libraries(plasma_appletscript_declarative PRIVATE
    plasma_appletscript_object
)

set_target_properties(plasma_appletscript_declarative PROPERTIES PREFIX "")
if (QT_MAJOR_VERSION EQUAL "6")
    target_link_libraries(plasma_appletscript_declarative PRIVATE KF5::ConfigQml)
endif()


install(TARGETS plasma_appletscript_object ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})
install(TARGETS plasma_appletscript_declarative DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/scriptengines)
install(FILES data/plasma-wallpaper.desktop DESTINATION ${KDE_INSTALL_KSERVICETYPESDIR})

# QML PLUGIN
ecm_add_qml_module(plasmoidplugin URI "org.kde.plasma.plasmoid" VERSION 2.0 CLASSNAME plasmoidplugin)

target_sources(plasmoidplugin PRIVATE
    plasmoid/plasmoidplugin.cpp
)

target_link_libraries(plasmoidplugin PRIVATE
    plasma_appletscript_object
)

install(TARGETS plasmoidplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/plasmoid)

ecm_finalize_qml_module(plasmoidplugin DESTINATION ${KDE_INSTALL_QMLDIR})
ecm_generate_qmltypes(org.kde.plasma.plasmoid 2.0 DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/plasmoid)
