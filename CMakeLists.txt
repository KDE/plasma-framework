find_package(KDE4 REQUIRED)
include (KDE4Defaults)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs ${CMAKE_CURRENT_SOURCE_DIR}/.. ${KDE4_INCLUDES} ${OPENGL_INCLUDE_DIR})

add_subdirectory(widgets)
#add_subdirectory(tests)
add_definitions(-DKDE_DEFAULT_DEBUG_AREA=1209)

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
    appletbrowser.cpp
    appletbrowser/customdragtreeview.cpp
    appletbrowser/kcategorizeditemsview.cpp
    appletbrowser/kcategorizeditemsviewdelegate.cpp
    appletbrowser/kcategorizeditemsviewmodels.cpp
    appletbrowser/openwidgetassistant.cpp
    appletbrowser/plasmaappletitemmodel.cpp
    applethandle.cpp
    configxml.cpp
    containment.cpp
    corona.cpp
    datacontainer.cpp
    dataengine.cpp
    dataenginemanager.cpp
    dialog.cpp
    #FOR FUTURE
    #layouts/borderlayout.cpp
    packages.cpp
    phase.cpp
    plasma.cpp
    plasma_export.h
    searchmatch.cpp
    searchcontext.cpp
    shadowitem.cpp
    svg.cpp
    svgpanel.cpp
    theme.cpp
    timer.cpp
    desktoptoolbox.cpp
    uiloader.cpp
    view.cpp
    delegate.cpp
    scripting/appletscript.cpp
    scripting/dataenginescript.cpp
    scripting/runnerscript.cpp
    scripting/scriptengine.cpp
    widgets/icon.cpp
    widgets/tooltip.cpp
    widgets/widget.cpp
    widgets/webcontent.cpp
)

kde4_add_ui_files (
    plasma_LIB_SRCS
    appletbrowser/kcategorizeditemsviewbase.ui
)

if(QT_QTOPENGL_FOUND AND OPENGL_FOUND)
MESSAGE(STATUS "Adding support for OpenGL applets to libplasma")
set(plasma_LIB_SRCS
    ${plasma_LIB_SRCS}
    glapplet.cpp)
endif(QT_QTOPENGL_FOUND AND OPENGL_FOUND)

kde4_add_library(plasma SHARED ${plasma_LIB_SRCS})

target_link_libraries(plasma ${KDE4_KIO_LIBS} ${KDE4_KFILE_LIBS} ${QT_QTUITOOLS_LIBRARY} ${QT_QTWEBKIT_LIBRARY})

if(QT_QTOPENGL_FOUND AND OPENGL_FOUND)
    target_link_libraries(plasma ${QT_QTOPENGL_LIBRARY} ${OPENGL_gl_LIBRARY})
endif(QT_QTOPENGL_FOUND AND OPENGL_FOUND)

set_target_properties(plasma PROPERTIES VERSION 1.0.0 SOVERSION 1)
install(TARGETS plasma ${INSTALL_TARGETS_DEFAULT_ARGS})


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
    appletbrowser.h
    configxml.h
    containment.h
    corona.h
    datacontainer.h
    dataengine.h
    dataenginemanager.h
    dialog.h
    phase.h
    plasma.h
    plasma_export.h
    searchmatch.h
    searchcontext.h
    shadowitem_p.h
    svg.h
    svgpanel.h
    theme.h
    timer.h
    uiloader.h
    view.h
    delegate.h)

if(QT_QTOPENGL_FOUND AND OPENGL_FOUND)
set(plasma_LIB_INCLUDES
    ${plasma_LIB_INCLUDES}
    glapplet.h)
endif(QT_QTOPENGL_FOUND AND OPENGL_FOUND)

install(FILES
        ${plasma_LIB_INCLUDES}
        DESTINATION ${INCLUDE_INSTALL_DIR}/plasma)

install(FILES
    widgets/icon.h
    widgets/webcontent.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/plasma/widgets)

#For future
#install(FILES
#    layouts/borderlayout.h
#    DESTINATION ${INCLUDE_INSTALL_DIR}/plasma/layouts)

install(FILES
    scripting/appletscript.h
    scripting/dataenginescript.h
    scripting/runnerscript.h
    scripting/scriptengine.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/plasma/scripting)


install(FILES
    includes/AbstractRunner
    includes/Animator
    includes/Applet
    includes/AppletBrowser
    includes/ConfigXml
    includes/Corona
    includes/Containment
    includes/Dialog
    includes/Phase
    includes/Plasma
    includes/Package
    includes/PackageStructure
    includes/Theme
    includes/DataContainer
    includes/DataEngine
    includes/DataEngineManager
    includes/SearchContext
    includes/SearchMatch
    includes/Svg
    includes/SvgPanel
    includes/UiLoader
    includes/PackageMetadata
    includes/View
    includes/Delegate
    DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/Plasma)

if(QT_QTOPENGL_FOUND AND OPENGL_FOUND)
install(FILES
    includes/GLApplet
    DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/Plasma)
endif(QT_QTOPENGL_FOUND AND OPENGL_FOUND)

install(FILES
    includes/ScriptEngine
    includes/DataEngineScript 
    includes/RunnerScript 
    includes/AppletScript
    DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/Plasma/Scripting)


install(FILES
    servicetypes/plasma-animator.desktop
    servicetypes/plasma-applet.desktop
    servicetypes/plasma-containment.desktop
    servicetypes/plasma-dataengine.desktop
    servicetypes/plasma-packagestructure.desktop
    servicetypes/plasma-runner.desktop
    servicetypes/plasma-scriptengine.desktop
    DESTINATION ${SERVICETYPES_INSTALL_DIR})

