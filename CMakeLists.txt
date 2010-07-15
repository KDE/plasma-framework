# strange conversion error in pushbutton.cpp
kde4_no_enable_final(plasma)

if(KDE_PLATFORM_FEATURE_BINARY_COMPATIBLE_FEATURE_REDUCTION)
    set(PLASMA_NO_KDEWEBKIT TRUE)
    set(PLASMA_NO_KNEWSTUFF TRUE)
    set(PLASMA_NO_SOLID TRUE)
    # set(PLASMA_NO_KIO TRUE)
endif(KDE_PLATFORM_FEATURE_BINARY_COMPATIBLE_FEATURE_REDUCTION)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${KDE4_KDECORE_INCLUDES}
                    ${KDE4_KDEUI_INCLUDES}
                    ${CMAKE_SOURCE_DIR}/threadweaver/
                    ${CMAKE_SOURCE_DIR}/plasma/extenders
                    ${CMAKE_SOURCE_DIR}/plasma/remote
                    ${CMAKE_SOURCE_DIR}/plasma/runners
                    ${CMAKE_SOURCE_DIR}/plasma/private/qtjolie-branch/qtjolie
                    ${CMAKE_SOURCE_DIR}/plasma/private/qtjolie-branch
                    ${CMAKE_SOURCE_DIR}/plasma/private/qtjolie-branch/includes
                    ${KDE4_INCLUDES}
                    )

if(NOT PLASMA_NO_KIO)
   include_directories(${KDE4_KIO_INCLUDES})
   set(PLASMA_EXTRA_LIBS ${PLASMA_EXTRA_LIBS} ${KDE4_KIO_LIBS})
endif(NOT PLASMA_NO_KIO)

if(NOT PLASMA_NO_KDEWEBKIT)
    include_directories(${CMAKE_SOURCE_DIR}/kdewebkit/)
    set(PLASMA_EXTRA_LIBS ${PLASMA_EXTRA_LIBS} kdewebkit)
endif(NOT PLASMA_NO_KDEWEBKIT)

if(NOT PLASMA_NO_KNEWSTUFF)
    include_directories(${CMAKE_SOURCE_DIR}/knewstuff/)
    set(PLASMA_EXTRA_LIBS ${PLASMA_EXTRA_LIBS} knewstuff3)
endif(NOT PLASMA_NO_KNEWSTUFF)

if(NOT PLASMA_NO_SOLID)
    include_directories(${CMAKE_BINARY_DIR}/solid/)
    include_directories(${CMAKE_SOURCE_DIR}/solid/)
    set(PLASMA_EXTRA_LIBS ${PLASMA_EXTRA_LIBS} ${KDE4_SOLID_LIBS})
endif(NOT PLASMA_NO_SOLID)

if(QCA2_FOUND)
    include_directories(${QCA2_INCLUDE_DIR})
    set(ENABLE_REMOTE_WIDGETS TRUE)
endif(QCA2_FOUND)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-plasma.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-plasma.h)

if(QT_QTOPENGL_FOUND AND OPENGL_FOUND)
    # libGL needs dlopen() and friends from the dl library
    find_library(DL_LIBRARY dl)
    mark_as_advanced(DL_LIBRARY)
    include_directories(${OPENGL_INCLUDE_DIR})
endif(QT_QTOPENGL_FOUND AND OPENGL_FOUND)

add_subdirectory(tests)
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
    abstracttoolbox.cpp
    animator.cpp
    animations/animation.cpp
    animations/animationscriptengine.cpp
    animations/bindings/animationgroup.cpp
    animations/bindings/easingcurve.cpp
    animations/easinganimation.cpp
    animations/fade.cpp
    animations/grow.cpp
    animations/javascriptanimation.cpp
    animations/pendulumcurve.cpp
    animations/pixmaptransition.cpp
    animations/pulser.cpp
    animations/rotation.cpp
    animations/rotationstacked.cpp
    animations/slide.cpp
    animations/stackedlayout.cpp
    animations/geometry.cpp
    animations/water.cpp
    animations/widgetsnapshot.cpp
    animations/zoom.cpp
    applet.cpp
    configloader.cpp
    containment.cpp
    containmentactions.cpp
    containmentactionspluginsconfig.cpp
    context.cpp
    corona.cpp
    datacontainer.cpp
    dataengine.cpp
    dataenginemanager.cpp
    delegate.cpp
    deprecated/animationdriver.cpp
    deprecated/animator.cpp
    dialog.cpp
    abstractdialogmanager.cpp
    extenders/extender.cpp
    extenders/extendergroup.cpp
    extenders/extenderitem.cpp
    pluginloader.cpp
    paintutils.cpp
    framesvg.cpp
    plasma.cpp
    popupapplet.cpp
    private/animablegraphicswebview.cpp
    private/applethandle.cpp
    private/associatedapplicationmanager.cpp
    private/datacontainer_p.cpp
    private/dataengineconsumer.cpp
    private/dataengineservice.cpp
    private/denyallauthorization.cpp
    private/desktoptoolbox.cpp
    private/extenderapplet.cpp
    private/extenderitemmimedata.cpp
    private/focusindicator.cpp
    private/getsource.cpp
    private/nativetabbar.cpp
    private/packages.cpp
    private/paneltoolbox.cpp
    private/pinpairingauthorization.cpp
    private/pinpairingdialog.cpp
    private/plasmoidservice.cpp
    private/remotedataengine.cpp
    private/remoteservice.cpp
    private/remoteservicejob.cpp
    private/runnerjobs.cpp
    private/serviceprovider.cpp
    private/storage.cpp
    private/style.cpp
    private/trustedonlyauthorization.cpp
    private/internaltoolbox.cpp
    private/tooltip.cpp
    private/wallpaperrenderthread.cpp
    private/windowpreview.cpp
    private/kineticscroll.cpp
    private/effects/halopainter.cpp
    private/effects/ripple.cpp
    querymatch.cpp
    remote/accessmanager.cpp
    remote/accessappletjob.cpp
    remote/authorizationinterface.cpp
    remote/authorizationmanager.cpp
    remote/authorizationrule.cpp
    remote/clientpinrequest.cpp
    remote/credentials.cpp
    remote/serviceaccessjob.cpp
    runnercontext.cpp
    runnermanager.cpp
    runnersyntax.cpp
    scripting/appletscript.cpp
    scripting/dataenginescript.cpp
    scripting/runnerscript.cpp
    scripting/wallpaperscript.cpp
    scripting/scriptengine.cpp
    service.cpp
    servicejob.cpp
    svg.cpp
    theme.cpp
    tooltipcontent.cpp
    tooltipmanager.cpp
    version.cpp
    view.cpp
    wallpaper.cpp
    windoweffects.cpp
    widgets/checkbox.cpp
    widgets/combobox.cpp
    widgets/flashinglabel.cpp
    widgets/frame.cpp
    widgets/groupbox.cpp
    widgets/iconwidget.cpp
    widgets/itembackground.cpp
    widgets/label.cpp
    widgets/lineedit.cpp
    widgets/meter.cpp
    widgets/pushbutton.cpp
    widgets/radiobutton.cpp
    widgets/scrollbar.cpp
    widgets/signalplotter.cpp
    widgets/slider.cpp
    widgets/spinbox.cpp
    widgets/toolbutton.cpp
    widgets/busywidget.cpp
    widgets/scrollwidget.cpp
    widgets/separator.cpp
    widgets/svgwidget.cpp
    widgets/tabbar.cpp
    widgets/textbrowser.cpp
    widgets/treeview.cpp
    widgets/textedit.cpp
    widgets/webview.cpp

    #Temporary QtJolie branch
    private/qtjolie-branch/qtjolie/abstractadaptor.cpp
    private/qtjolie-branch/qtjolie/client.cpp
    private/qtjolie-branch/qtjolie/clientthread.cpp
    private/qtjolie-branch/qtjolie/value.cpp
    private/qtjolie-branch/qtjolie/fault.cpp
    private/qtjolie-branch/qtjolie/message.cpp
    private/qtjolie-branch/qtjolie/metaservice.cpp
    private/qtjolie-branch/qtjolie/pendingcall.cpp
    private/qtjolie-branch/qtjolie/pendingcallwatcher.cpp
    private/qtjolie-branch/qtjolie/pendingreply.cpp
    private/qtjolie-branch/qtjolie/server.cpp
    private/qtjolie-branch/qtjolie/serverthread.cpp
)

kde4_add_kcfg_files(plasma_LIB_SRCS data/kconfigxt/libplasma-theme-global.kcfgc)

kde4_add_ui_files(plasma_LIB_SRCS
                  private/pinpairing.ui
                  private/publish.ui)
#NEPOMUK_GENERATE_FROM_ONTOLOGY(
#   nwc.nrl
#   ${metadata_test_BINARY_DIR}
#   TEST_HEADERS
#   TEST_SOURCES
#   TEST_INCLUDES
#)

if(QT_QTOPENGL_FOUND AND OPENGL_FOUND)
    message(STATUS "Adding support for OpenGL applets to libplasma")
    set(plasma_LIB_SRCS
        ${plasma_LIB_SRCS}
        glapplet.cpp)
endif(QT_QTOPENGL_FOUND AND OPENGL_FOUND)

if (PHONON_FOUND)
    message(STATUS "Adding support for Phonon to libplasma")
    set(plasma_LIB_SRCS
        ${plasma_LIB_SRCS}
        widgets/videowidget.cpp)
endif(PHONON_FOUND)

kde4_add_library(plasma SHARED ${plasma_LIB_SRCS})

target_link_libraries(plasma ${QT_QTUITOOLS_LIBRARY} ${QT_QTWEBKIT_LIBRARY}
                             ${QT_QTSCRIPT_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTXML_LIBRARY}
                             ${KDE4_KDEUI_LIBS} kdnssd threadweaver ${PLASMA_EXTRA_LIBS})

if(QCA2_FOUND)
   target_link_libraries(plasma ${QCA2_LIBRARIES})
endif(QCA2_FOUND)

if(X11_FOUND)
   target_link_libraries(plasma ${X11_LIBRARIES})
endif(X11_FOUND)

if(PHONON_FOUND)
   target_link_libraries(plasma ${KDE4_PHONON_LIBS})
endif(PHONON_FOUND)

if(DL_LIBRARY)
    target_link_libraries(plasma ${DL_LIBRARY})
endif(DL_LIBRARY)

if(QT_QTOPENGL_FOUND AND OPENGL_FOUND)
    target_link_libraries(plasma ${QT_QTOPENGL_LIBRARY} ${OPENGL_gl_LIBRARY})
endif(QT_QTOPENGL_FOUND AND OPENGL_FOUND)

target_link_libraries(plasma LINK_INTERFACE_LIBRARIES kdeui kdecore ${QT_QTGUI_LIBRARY})

#do NOT use GENERIC versioning -- the plasma team will take care of versioning
set_target_properties(plasma PROPERTIES
                             VERSION 3.0.0
                             SOVERSION 3
                      )

install(TARGETS plasma EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

set(plasmagik_HEADERS
    packagemetadata.h
    packagestructure.h
    package.h
    )

install(FILES ${plasmagik_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR}/plasma/ COMPONENT Devel)

set(plasma_LIB_INCLUDES
    abstractdialogmanager.h
    abstractrunner.h
    abstracttoolbox.h
    animationdriver.h
    animations/animation.h
    animator.h
    applet.h
    configloader.h
    containment.h
    containmentactions.h
    containmentactionspluginsconfig.h
    context.h
    corona.h
    datacontainer.h
    dataengine.h
    dataenginemanager.h
    delegate.h
    dialog.h
    extenders/extender.h
    extenders/extendergroup.h
    extenders/extenderitem.h
    pluginloader.h
    paintutils.h
    windoweffects.h
    framesvg.h
    plasma.h
    plasma_export.h
    popupapplet.h
    querymatch.h
    remote/accessappletjob.h
    remote/accessmanager.h
    remote/authorizationmanager.h
    remote/authorizationrule.h
    remote/authorizationinterface.h
    remote/clientpinrequest.h
    remote/credentials.h
    remote/serviceaccessjob.h
    runnercontext.h
    runnermanager.h
    runnersyntax.h
    service.h
    servicejob.h
    svg.h
    theme.h
    tooltipcontent.h
    tooltipmanager.h
    version.h
    view.h
    wallpaper.h)

if(QT_QTOPENGL_FOUND AND OPENGL_FOUND)
    set(plasma_LIB_INCLUDES
        ${plasma_LIB_INCLUDES}
        glapplet.h)
endif(QT_QTOPENGL_FOUND AND OPENGL_FOUND)

install(FILES
        ${plasma_LIB_INCLUDES}
        DESTINATION ${INCLUDE_INSTALL_DIR}/plasma COMPONENT Devel)

install(FILES
    widgets/checkbox.h
    widgets/combobox.h
    widgets/flashinglabel.h
    widgets/frame.h
    widgets/groupbox.h
    widgets/iconwidget.h
    widgets/itembackground.h
    widgets/label.h
    widgets/lineedit.h
    widgets/meter.h
    widgets/pushbutton.h
    widgets/toolbutton.h
    widgets/radiobutton.h
    widgets/scrollbar.h
    widgets/signalplotter.h
    widgets/slider.h
    widgets/spinbox.h
    widgets/busywidget.h
    widgets/separator.h
    widgets/svgwidget.h
    widgets/scrollwidget.h
    widgets/tabbar.h
    widgets/textbrowser.h
    widgets/treeview.h
    widgets/textedit.h
    widgets/webview.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/plasma/widgets COMPONENT Devel)

install(FILES
    scripting/appletscript.h
    scripting/dataenginescript.h
    scripting/runnerscript.h
    scripting/wallpaperscript.h
    scripting/scriptengine.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/plasma/scripting COMPONENT Devel)

if(PHONON_FOUND)
    install (FILES
        widgets/videowidget.h
        DESTINATION ${INCLUDE_INSTALL_DIR}/plasma/widgets COMPONENT Devel)
endif(PHONON_FOUND)

install(FILES
    animations/animation.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/plasma/animations COMPONENT Devel)


install(FILES
   data/servicetypes/plasma-animator.desktop
   data/servicetypes/plasma-applet.desktop
   data/servicetypes/plasma-applet-popupapplet.desktop
   data/servicetypes/plasma-containment.desktop
   data/servicetypes/plasma-containmentactions.desktop
   data/servicetypes/plasma-dataengine.desktop
   data/servicetypes/plasma-packagestructure.desktop
   data/servicetypes/plasma-runner.desktop
   data/servicetypes/plasma-scriptengine.desktop
   data/servicetypes/plasma-wallpaper.desktop
   DESTINATION ${SERVICETYPES_INSTALL_DIR})

install(FILES
   data/services/plasma-applet-extenderapplet.desktop
   DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES data/knewstuff/plasmoids.knsrc DESTINATION  ${CONFIG_INSTALL_DIR})

install(FILES data/kconfig_updates/plasma_popupapplet_fix_groups.upd DESTINATION ${KCONF_UPDATE_INSTALL_DIR})
install(PROGRAMS data/kconfig_updates/plasma_popupapplet_fix_groups.pl DESTINATION ${KCONF_UPDATE_INSTALL_DIR})
install(FILES data/operations/dataengineservice.operations DESTINATION ${DATA_INSTALL_DIR}/plasma/services)
install(FILES data/operations/plasmoidservice.operations DESTINATION ${DATA_INSTALL_DIR}/plasma/services)
install(FILES data/operations/storage.operations DESTINATION ${DATA_INSTALL_DIR}/plasma/services)

