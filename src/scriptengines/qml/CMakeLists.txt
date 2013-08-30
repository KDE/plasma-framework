# APPLET

if(KDE_PLATFORM_FEATURE_BINARY_COMPATIBLE_FEATURE_REDUCTION)
    set(PLASMA_NO_KDEWEBKIT TRUE)
    set(PLASMA_NO_KNEWSTUFF TRUE)
    set(PLASMA_NO_SOLID TRUE)
    set(PLASMA_NO_KIO TRUE)
endif(KDE_PLATFORM_FEATURE_BINARY_COMPATIBLE_FEATURE_REDUCTION)

include_directories(${KDE4_INCLUDE_DIR}/KDE ${PHONON_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR}/common)



#DECLARATIVE APPLET
set(declarative_appletscript_SRCS
    declarative/packageaccessmanager.cpp
    declarative/packageaccessmanagerfactory.cpp
    plasmoid/appletinterface.cpp
    plasmoid/containmentinterface.cpp
    plasmoid/declarativeappletscript.cpp
    plasmoid/wallpaperinterface.cpp
    )

include_directories(${PHONON_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR}/common)

add_library(plasma_appletscript_declarative MODULE ${declarative_appletscript_SRCS} )
set_target_properties(plasma_appletscript_declarative PROPERTIES PREFIX "")

target_link_libraries(plasma_appletscript_declarative
    ${KDE4_KIO_LIBS}
    ${KDE4Support_LIBRARIES}
    ${KDE4Attic_LIBRARIES}
    ${QT_QTSCRIPT_LIBRARY}
    ${Qt5Quick_LIBRARIES}
    ${Qt5Qml_LIBRARIES}
    ${QT_QTUITOOLS_LIBRARY}
    ${KCore_LIBRARIES}
    KF5::KI18n
    plasma
    kdeclarative
)


install(TARGETS plasma_appletscript_declarative DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES data/plasma-scriptengine-applet-declarative.desktop DESTINATION ${SERVICES_INSTALL_DIR})


