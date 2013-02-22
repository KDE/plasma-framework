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
    declarative/configpropertymap.cpp
    declarative/packageaccessmanager.cpp
    declarative/packageaccessmanagerfactory.cpp
    declarative/qmlobject.cpp
    plasmoid/appletinterface.cpp
    plasmoid/configview.cpp
    plasmoid/containmentinterface.cpp
    plasmoid/declarativeappletscript.cpp
    )

include_directories(${PHONON_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR}/common)

kde4_add_plugin(plasma_appletscript_declarative ${declarative_appletscript_SRCS})
target_link_libraries(plasma_appletscript_declarative
    ${KDE4_KIO_LIBS}
    ${KDE4_KDE4SUPPORT_LIBS}
    ${QT_QTSCRIPT_LIBRARY}
    ${Qt5Quick_LIBRARIES}
    ${Qt5Qml_LIBRARIES}
    ${QT_QTUITOOLS_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    plasma
    kdeclarative
)


install(TARGETS plasma_appletscript_declarative DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES data/plasma-scriptengine-applet-declarative.desktop DESTINATION ${SERVICES_INSTALL_DIR})


