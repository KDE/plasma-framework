find_package(KDE4 REQUIRED)
include (KDE4Defaults)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs ${CMAKE_CURRENT_SOURCE_DIR}/.. ${KDE4_INCLUDES} ${OPENGL_INCLUDE_DIR})

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
    animationdriver.cpp
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
    delegate.cpp
    desktoptoolbox.cpp
    dialog.cpp
    #FOR FUTURE
    #layouts/borderlayout.cpp
    packages.cpp
    panelsvg.cpp
    paneltoolbox.cpp
    plasma.cpp
    plasma_export.h
    querymatch.cpp
    runnercontext.cpp
    runnermanager.cpp
    scripting/appletscript.cpp
    scripting/dataenginescript.cpp
    scripting/runnerscript.cpp
    scripting/scriptengine.cpp
    service.cpp
    servicejob.cpp
    shadowitem.cpp
    svg.cpp
    theme.cpp
    toolbox.cpp
    uiloader.cpp
    view.cpp
    widgets/checkbox.cpp
    widgets/combobox.cpp
    widgets/flash.cpp
    widgets/groupbox.cpp
    widgets/icon.cpp
    widgets/label.cpp
    widgets/lineedit.cpp
    widgets/meter.cpp
    widgets/pushbutton.cpp
    widgets/radiobutton.cpp
    widgets/signalplotter.cpp
    widgets/textedit.cpp
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

target_link_libraries(plasma ${KDE4_KIO_LIBS} ${KDE4_KFILE_LIBS}
                             ${QT_QTUITOOLS_LIBRARY} ${QT_QTWEBKIT_LIBRARY}
                             ${KDE4_THREADWEAVER_LIBRARIES})

if(QT_QTOPENGL_FOUND AND OPENGL_FOUND)
    target_link_libraries(plasma ${QT_QTOPENGL_LIBRARY} ${OPENGL_gl_LIBRARY})
endif(QT_QTOPENGL_FOUND AND OPENGL_FOUND)

set_target_properties(plasma PROPERTIES VERSION 2.0.0 SOVERSION 2)
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
    animationdriver.h
    animator.h
    applet.h
    appletbrowser.h
    configxml.h
    containment.h
    corona.h
    datacontainer.h
    dataengine.h
    dataenginemanager.h
    delegate.h
    dialog.h
    panelsvg.h
    plasma.h
    plasma_export.h
    querymatch.h
    runnercontext.h
    runnermanager.h
    service.h
    servicejob.h
    svg.h
    theme.h
    uiloader.h
    view.h)

if(QT_QTOPENGL_FOUND AND OPENGL_FOUND)
set(plasma_LIB_INCLUDES
    ${plasma_LIB_INCLUDES}
    glapplet.h)
endif(QT_QTOPENGL_FOUND AND OPENGL_FOUND)

install(FILES
        ${plasma_LIB_INCLUDES}
        DESTINATION ${INCLUDE_INSTALL_DIR}/plasma)

install(FILES
    widgets/checkbox.h
    widgets/combobox.h
    widgets/flash.h
    widgets/groupbox.h
    widgets/icon.h
    widgets/label.h
    widgets/lineedit.h
    widgets/meter.h
    widgets/pushbutton.h
    widgets/radiobutton.h
    widgets/signalplotter.h
    widgets/textedit.h
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
    includes/AnimationDriver
    includes/Animator
    includes/Applet
    includes/AppletBrowser
    includes/ConfigXml
    includes/Corona
    includes/Containment
    includes/Dialog
    includes/Plasma
    includes/Package
    includes/PackageStructure
    includes/Theme
    includes/DataContainer
    includes/DataEngine
    includes/DataEngineManager
    includes/RunnerContext
    includes/RunnerManager
    includes/QueryMatch
    includes/Service
    includes/ServiceJob
    includes/Svg
    includes/PanelSvg
    includes/UiLoader
    includes/PackageMetadata
    includes/View
    includes/Delegate
    DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/Plasma)

install(FILES
   includes/Label
   includes/LineEdit
   includes/PushButton
   includes/RadioButton
   includes/WebContent
   DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/Plasma/Widgets)


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

