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
    Qt6::Quick
    Qt6::Qml
    KF6::Activities
    KF6::ConfigQml
    KF6::KIOCore
    KF6::KIOWidgets
    KF6::I18n
    KF6::XmlGui # KActionCollection
    KF6::Plasma
    KF6::PlasmaQuick
    KF6::Package
    KF6::Notifications
)

install(TARGETS plasma_appletscript_declarative DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/scriptengines)
