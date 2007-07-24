include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs)

########### next target ###############

set(plasmagik_SRCS
    packagemetadata.cpp
    packagestructure.cpp
    package.cpp
    )

set(plasma_LIB_SRCS
    ${plasmagik_SRCS}
    abstractrunner.cpp
    animator.cpp
    applet.cpp
    configxml.cpp
    corona.cpp
    datacontainer.cpp
    dataengine.cpp
    dataenginemanager.cpp
    packages.cpp
    phase.cpp
    plasma.cpp
    plasma_export.h
    scriptengine.cpp
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
    widgets/flash.cpp
   )

if(QT_QTOPENGL_FOUND)
set(plasma_LIB_SRCS
    ${plasma_LIB_SRCS}
    glapplet.cpp)
endif(QT_QTOPENGL_FOUND)

kde4_add_library(plasma SHARED ${plasma_LIB_SRCS})

target_link_libraries(plasma ${KDE4_KIO_LIBS} ${QT_QTOPENGL_LIBRARY} ${OPENGL_gl_LIBRARY})

set_target_properties(plasma PROPERTIES VERSION 1.0.0 SOVERSION 1)
install(TARGETS plasma DESTINATION ${LIB_INSTALL_DIR})


########### install files ###############

set(plasmagik_HEADERS
    packagemetadata.h
    packagestructure.h
    package.h
    )

install(FILES ${plasmagik_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR}/plasma/)

set(plasma_LIB_INCLUDES
    abstractrunner.h
    animator.h
    applet.h
    configxml.h
    corona.h
    datacontainer.h
    dataengine.h
    dataenginemanager.h
    phase.h
    plasma.h
    plasma_export.h
    scriptengine.cpp
    svg.h
    theme.h)

if(QT_QTOPENGL_FOUND)
set(plasma_LIB_INCLUDES
    glapplet.h)
endif(QT_QTOPENGL_FOUND)

install(FILES
        ${plasma_LIB_INCLUDES}
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
    widgets/flash.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/plasma/widgets)

install(FILES
    includes/AbstractRunner
    includes/Applet
    includes/ConfigXml
    includes/Phase
    includes/Plasma
    includes/Package
    includes/Packager
    includes/PackageStructure
    includes/Theme
    includes/DataContainer
    includes/DataEngine
    includes/DataEngineManager
    includes/ScriptEngine
    includes/Svg
    DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/Plasma)

install(FILES
    servicetypes/plasma-animator.desktop
    servicetypes/plasma-applet.desktop
    servicetypes/plasma-dataengine.desktop
    servicetypes/plasma-scriptengine.desktop
    DESTINATION ${SERVICETYPES_INSTALL_DIR})

