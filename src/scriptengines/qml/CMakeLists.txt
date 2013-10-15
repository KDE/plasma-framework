# APPLET

if(KDE_PLATFORM_FEATURE_BINARY_COMPATIBLE_FEATURE_REDUCTION)
    set(PLASMA_NO_KDEWEBKIT TRUE)
    set(PLASMA_NO_KNEWSTUFF TRUE)
    set(PLASMA_NO_SOLID TRUE)
    set(PLASMA_NO_KIO TRUE)
endif(KDE_PLATFORM_FEATURE_BINARY_COMPATIBLE_FEATURE_REDUCTION)

#DECLARATIVE APPLET
set(declarative_appletscript_SRCS
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
    Qt5::UiTools
    KF5::KIOCore
    KF5::KIOWidgets	
    KF5::KDE4Attic
    KF5::KI18n
    plasma
    PlasmaQuick
    KF5::KDeclarative
)


install(TARGETS plasma_appletscript_declarative DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES data/plasma-scriptengine-applet-declarative.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES data/plasma-wallpaper.desktop DESTINATION ${SERVICES_INSTALL_DIR})


