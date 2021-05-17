# APPLET

if(KDE_PLATFORM_FEATURE_BINARY_COMPATIBLE_FEATURE_REDUCTION)
    set(PLASMA_NO_KDEWEBKIT TRUE)
    set(PLASMA_NO_SOLID TRUE)
endif()

#DECLARATIVE APPLET
add_library(plasma_appletscript_declarative MODULE)

target_sources(plasma_appletscript_declarative PRIVATE
    plasmoid/declarativeappletscript.cpp
    plasmoid/dropmenu.cpp
    plasmoid/appletinterface.cpp
    plasmoid/containmentinterface.cpp
    plasmoid/wallpaperinterface.cpp
)

set_target_properties(plasma_appletscript_declarative PROPERTIES PREFIX "")

kcoreaddons_desktop_to_json(
    plasma_appletscript_declarative data/plasma-scriptengine-applet-declarative.desktop
    SERVICE_TYPES ${CMAKE_SOURCE_DIR}/src/plasma/data/servicetypes/plasma-scriptengine.desktop
)


target_link_libraries(plasma_appletscript_declarative
    Qt5::Quick
    Qt5::Qml
    KF5::Activities
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


install(TARGETS plasma_appletscript_declarative DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/scriptengines)
install(FILES data/plasma-scriptengine-applet-declarative.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(FILES data/plasma-wallpaper.desktop DESTINATION ${KDE_INSTALL_KSERVICETYPES5DIR})


