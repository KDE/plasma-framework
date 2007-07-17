include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs)

########### next target ###############

set(plasmagik_SRCS
    packagemetadata.cpp
    packager.cpp
    packagestructure.cpp
    package.cpp
    )

set(plasma_LIB_SRCS
    ${plasmagik_SRCS}
    abstractrunner.cpp
    animator.cpp
    applet.cpp
    corona.cpp
    dataengine.cpp
    dataenginemanager.cpp
    datasource.cpp
    phase.cpp
    plasma.cpp
    plasma_export.h
    svg.cpp
    theme.cpp
    karambamanager.cpp
    widgets/checkbox.cpp
    widgets/icon.cpp
    widgets/lineedit.cpp
    widgets/pushbutton.cpp
    widgets/radiobutton.cpp
    widgets/widget.cpp
    widgets/layout.cpp
    widgets/layoutitem.cpp
    widgets/vboxlayout.cpp
   )

set(plasmagik_HEADERS
    packagemetadata.h
    packager.h
    packagestructure.h
    package.h
    )

set(krunner_xml ${CMAKE_SOURCE_DIR}/workspace/krunner/org.kde.krunner.Interface.xml)
QT4_ADD_DBUS_INTERFACE(plasma_LIB_SRCS ${krunner_xml} krunner_interface)

kde4_add_library(plasma SHARED ${plasma_LIB_SRCS})

target_link_libraries(plasma ${KDE4_KIO_LIBS})

set_target_properties(plasma PROPERTIES VERSION 1.0.0 SOVERSION 1)
install(TARGETS plasma DESTINATION ${LIB_INSTALL_DIR})


########### install files ###############

install(FILES ${plasmagik_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR}/plasma/)

install(FILES
    abstractrunner.h
    animator.h
    applet.h
    corona.h
    dataengine.h
    dataenginemanager.h
    datasource.h
    phase.h
    plasma.h
    plasma_export.h
    svg.h
    theme.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/plasma)

install(FILES
    widgets/icon.h
    widgets/layout.h
    widgets/layoutitem.h
    widgets/lineedit.h
    widgets/pushbutton.h
    widgets/checkbox.h
    widgets/radiobutton.h
    widgets/vboxlayout.h
    widgets/widget.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/plasma/widgets)

install(FILES
    includes/Applet
    includes/Phase
    includes/Plasma
    includes/AbstractRunner
    includes/Theme
    includes/DataEngine
    includes/DataEngineManager
    includes/DataSource
    includes/Svg
    DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/Plasma)

install(FILES
    servicetypes/plasma_animator.desktop
    servicetypes/plasma_applet.desktop
    servicetypes/plasma_dataengine.desktop
    DESTINATION ${SERVICETYPES_INSTALL_DIR})

