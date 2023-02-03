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

target_link_libraries(plasma_appletscript_declarative
    Qt${QT_MAJOR_VERSION}::Quick
    Qt${QT_MAJOR_VERSION}::Qml
    KF6::Activities
    KF6::ConfigQml
    KF6::KIOCore
    KF6::KIOWidgets
    KF6::Declarative
    KF6::I18n
    KF6::XmlGui # KActionCollection
    KF6::Plasma
    KF6::PlasmaQuick
    KF6::Package
    KF6::Notifications
)
if (QT_MAJOR_VERSION EQUAL "6")
    target_link_libraries(plasma_appletscript_declarative KF6::ConfigQml)
endif()


install(TARGETS plasma_appletscript_declarative DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/scriptengines)
install(FILES data/plasma-wallpaper.desktop DESTINATION ${KDE_INSTALL_KSERVICETYPESDIR})


