# APPLET

if(KDE_PLATFORM_FEATURE_BINARY_COMPATIBLE_FEATURE_REDUCTION)
    set(PLASMA_NO_KDEWEBKIT TRUE)
    set(PLASMA_NO_SOLID TRUE)
    set(PLASMA_NO_KIO TRUE)
endif()

#DECLARATIVE APPLET
set(declarative_appletscript_SRCS
    plasmoid/declarativeappletscript.cpp

    plasmoid/appletinterface.cpp
    plasmoid/containmentinterface.cpp
    plasmoid/declarativeappletscript.cpp
    plasmoid/wallpaperinterface.cpp
    )

add_library(plasma_appletscript_declarative MODULE ${declarative_appletscript_SRCS} )
set_target_properties(plasma_appletscript_declarative PROPERTIES PREFIX "")

target_link_libraries(plasma_appletscript_declarative
    Qt5::Quick
    Qt5::Qml
    Qt5::Script
    KF5::Activities
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::Declarative
    KF5::I18n
    KF5::XmlGui # KActionCollection
    KF5::Plasma
    KF5::PlasmaQuick
    KF5::WindowSystem
)


install(TARGETS plasma_appletscript_declarative DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES data/plasma-scriptengine-applet-declarative.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES data/plasma-wallpaper.desktop DESTINATION ${SERVICES_INSTALL_DIR})


