
# This option should be removed, or moved down as far as possible.
# That means porting the existing frameworks to the CMake automoc
# feature. Porting is mostly removing explicit moc includes, and
# leaving the ones which are truly needed (ie, if you remove
# them, the build fails).
set(CMAKE_AUTOMOC_RELAXED_MODE ON)

if(KDE_PLATFORM_FEATURE_BINARY_COMPATIBLE_FEATURE_REDUCTION)
    set(PLASMA_NO_KNEWSTUFF TRUE)
    set(PLASMA_NO_SOLID TRUE)
    set(PLASMA_NO_KIO TRUE)
    set(PLASMA_NO_PACKAGEKIT TRUE)
    set(PLASMA_NO_PACKAGE_EXTRADATA TRUE)
    set(PLASMA_NO_KUTILS TRUE)
    set(PLASMA_NO_GLOBAL_SHORTCUTS TRUE)
endif(KDE_PLATFORM_FEATURE_BINARY_COMPATIBLE_FEATURE_REDUCTION)

if(NOT X11_FOUND)
    set(PLASMA_NO_PACKAGEKIT TRUE)
endif(NOT X11_FOUND)

#find_package(KdepimLibs 4.5.60)
#find_package(Gpgme)
#macro_log_feature(KDEPIMLIBS_FOUND "kdepimlibs" "KDE PIM libraries" "http://www.kde.org" FALSE "" "Needed for building several Plasma DataEngines")


include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${KDE4_KDECORE_INCLUDES}
                    ${KDE4_KDEUI_INCLUDES}
                    ${CMAKE_SOURCE_DIR}/experimental/libkdeclarative
                    ${CMAKE_BINARY_DIR}/experimental/libkdeclarative
                    ${karchive_SOURCE_DIR}/src
                    ${karchive_BINARY_DIR}/src
                    ${CMAKE_SOURCE_DIR}/tier2/kauth/src/
                    ${CMAKE_BINARY_DIR}/tier2/kauth/src/
                    ${CMAKE_SOURCE_DIR}/tier1/threadweaver/src
                    ${CMAKE_BINARY_DIR}/tier1/threadweaver/src
                    ${CMAKE_BINARY_DIR}/tier1/threadweaver/src/Weaver
                    ${CMAKE_SOURCE_DIR}/tier1/kwindowsystem/
                    ${CMAKE_BINARY_DIR}/tier1/kwindowsystem/
                    ${CMAKE_SOURCE_DIR}/tier1/solid/src
                    ${CMAKE_BINARY_DIR}/tier1/solid/src
                    ${CMAKE_SOURCE_DIR}/plasma/remote
                    ${CMAKE_SOURCE_DIR}/plasma/private/qtjolie-branch/qtjolie
                    ${CMAKE_SOURCE_DIR}/plasma/private/qtjolie-branch
                    ${CMAKE_SOURCE_DIR}/plasma/private/qtjolie-branch/includes
                    ${CMAKE_SOURCE_DIR}/tier1/kcoreaddons/src/caching
                    ${ki18n_SOURCE_DIR}/src
                    ${ki18n_BINARY_DIR}/src
                    )

if(NOT PLASMA_NO_KIO)
   include_directories(${KDE4_KIO_INCLUDES})
   set(PLASMA_EXTRA_LIBS ${PLASMA_EXTRA_LIBS} ${KDE4_KIO_LIBS})
endif(NOT PLASMA_NO_KIO)

if(NOT PLASMA_NO_KNEWSTUFF)
    include_directories(${CMAKE_SOURCE_DIR}/knewstuff/)
    include_directories(${CMAKE_BINARY_DIR}/knewstuff/)
    set(PLASMA_EXTRA_LIBS ${PLASMA_EXTRA_LIBS} knewstuff3)
endif(NOT PLASMA_NO_KNEWSTUFF)

if(NOT PLASMA_NO_SOLID)
    include_directories(${CMAKE_BINARY_DIR}/tier1/solid/)
    include_directories(${CMAKE_SOURCE_DIR}/tier1/solid/)
    set(PLASMA_EXTRA_LIBS ${PLASMA_EXTRA_LIBS} ${KDE4_SOLID_LIBS})
endif(NOT PLASMA_NO_SOLID)

if(NOT PLASMA_NO_PACKAGEKIT)
    add_definitions(-DPLASMA_ENABLE_PACKAGEKIT_SUPPORT=1)
    set(PLASMA_EXTRA_LIBS ${PLASMA_EXTRA_LIBS} ${QT_QTDBUS_LIBRARY})
endif(NOT PLASMA_NO_PACKAGEKIT)

if (NOT PLASMA_NO_KUTILS)
    include_directories(${CMAKE_SOURCE_DIR}/kutils)
    include_directories(${CMAKE_BINARY_DIR}/kutils)
    set(PLASMA_EXTRA_LIBS ${PLASMA_EXTRA_LIBS} ${KDE4_KUTILS_LIBS})
endif(NOT PLASMA_NO_KUTILS)

if(QCA2_FOUND)
    include_directories(${QCA2_INCLUDE_DIR})
    set(ENABLE_REMOTE_WIDGETS TRUE)
endif(QCA2_FOUND)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-plasma.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-plasma.h)

#FIXME: gpgme++ is in kdepimlibs, must move somewhere else!
include_directories(${KDEPIMLIBS_INCLUDE_DIRS} ${GPGME_INCLUDES})

add_subdirectory(tests)
add_definitions(-DKDE_DEFAULT_DEBUG_AREA=1209)

########### next target ###############

set(plasma_LIB_SRCS
    abstractrunner.cpp
    ${plasmagik_SRCS}
    abstractdialogmanager.cpp
    configloader.cpp
    containmentactions.cpp
    containmentactionspluginsconfig.cpp
    datacontainer.cpp
    dataengine.cpp
    dataengineconsumer.cpp
    package.cpp
    packagestructure.cpp
    paintutils.cpp
    pluginloader.cpp
    framesvg.cpp
    plasma.cpp
    pluginloader.cpp

    private/associatedapplicationmanager.cpp
    private/componentinstaller.cpp
    private/datacontainer_p.cpp
    private/dataenginemanager.cpp
    private/dataengineservice.cpp
    private/effects/halopainter.cpp
    private/getsource.cpp

    private/packages.cpp
    private/packagejob.cpp
    private/packagejobthread.cpp
    private/plasmoidservice.cpp
    private/remotedataengine.cpp
    private/remoteservice.cpp
    private/remoteservicejob.cpp
    private/runnerjobs.cpp
    private/serviceprovider.cpp
    private/storage.cpp
    private/storagethread.cpp
    private/windowpreview.cpp
    private/windowshadows.cpp
    private/applet_p.cpp
    private/containment_p.cpp

    querymatch.cpp
    remote/accessmanager.cpp
    remote/accessappletjob.cpp
    remote/authorizationinterface.cpp
    remote/authorizationmanager.cpp
    remote/authorizationrule.cpp
    remote/clientpinrequest.cpp
    remote/credentials.cpp
    remote/denyallauthorization.cpp
    remote/pinpairingauthorization.cpp
    remote/pinpairingdialog.cpp
    remote/serviceaccessjob.cpp
    #remote/signing.cpp TODO: re-enable, needs gpgmepp though
    remote/trustedonlyauthorization.cpp
    runnercontext.cpp
    runnermanager.cpp
    runnersyntax.cpp
    scripting/appletscript.cpp
    scripting/dataenginescript.cpp
    scripting/runnerscript.cpp
    scripting/scriptengine.cpp
    service.cpp
    servicejob.cpp
    svg.cpp
    theme.cpp
    version.cpp


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
    applet.cpp
    containment.cpp
    corona.cpp
)

if (QT5_BUILD)
  set(plasma_LIB_SRCS ${plasma_LIB_SRCS} private/effectwatcher.cpp)
endif()

kconfig_add_kcfg_files(plasma_LIB_SRCS data/kconfigxt/libplasma-theme-global.kcfgc)

kde4_add_ui_files(plasma_LIB_SRCS
                  remote/pinpairing.ui
                  private/publish.ui)
#NEPOMUK_GENERATE_FROM_ONTOLOGY(
#   nwc.nrl
#   ${metadata_test_BINARY_DIR}
#   TEST_HEADERS
#   TEST_SOURCES
#   TEST_INCLUDES
#)


kde4_add_library(plasma ${LIBRARY_TYPE} ${plasma_LIB_SRCS})

target_link_libraries(plasma ${QT_QTUITOOLS_LIBRARY}
                              ${QT_QTNETWORK_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTSQL_LIBRARY} 
                             ${KDE4_KDEUI_LIBS} kdnssd threadweaver kauth kwindowsystem ${PLASMA_EXTRA_LIBS} kcoreaddons ki18n)
#FIXME gpgme++ is in kdepimlibs, neeeds to be elsewhere
target_link_libraries(plasma ${KDEPIMLIBS_GPGMEPP_LIBS} kdeclarative karchive)

if(QCA2_FOUND)
   target_link_libraries(plasma ${QCA2_LIBRARIES})
endif(QCA2_FOUND)

if(X11_FOUND)
   target_link_libraries(plasma ${X11_LIBRARIES})
endif(X11_FOUND)

if(DL_LIBRARY)
    target_link_libraries(plasma ${DL_LIBRARY})
endif(DL_LIBRARY)

target_link_libraries(plasma LINK_INTERFACE_LIBRARIES kdeui kdecore kwindowsystem Qt5::Widgets)

#do NOT use GENERIC versioning -- the plasma team will take care of versioning
set_target_properties(plasma PROPERTIES
                             VERSION 4.0.0
                             SOVERSION 4
                      )

install(TARGETS plasma EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############
generate_export_header(plasma)

set(plasma_LIB_INCLUDES
    abstractdialogmanager.h
    abstractrunner.h
    applet.h
    configloader.h
    containment.h
    containmentactions.h
    containmentactionspluginsconfig.h
    corona.h
    datacontainer.h
    dataengine.h
    pluginloader.h
    paintutils.h
    framesvg.h
    package.h
    packagestructure.h
    plasma.h
    ${CMAKE_CURRENT_BINARY_DIR}/plasma_export.h
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
    version.h)


install(FILES
        ${plasma_LIB_INCLUDES}
        DESTINATION ${INCLUDE_INSTALL_DIR}/plasma COMPONENT Devel)


install(FILES
    scripting/appletscript.h
    scripting/dataenginescript.h
    scripting/runnerscript.h
    scripting/scriptengine.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/plasma/scripting COMPONENT Devel)

install(FILES
   data/servicetypes/plasma-applet.desktop
   data/servicetypes/plasma-containment.desktop
   data/servicetypes/plasma-containmentactions.desktop
   data/servicetypes/plasma-dataengine.desktop
   data/servicetypes/plasma-packagestructure.desktop
   data/servicetypes/plasma-runner.desktop
   data/servicetypes/plasma-scriptengine.desktop
   data/servicetypes/plasma-service.desktop
   DESTINATION ${SERVICETYPES_INSTALL_DIR})

install(FILES
   data/services/plasma.protocol
   DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES data/knewstuff/plasmoids.knsrc DESTINATION  ${CONFIG_INSTALL_DIR})

install(FILES data/operations/dataengineservice.operations DESTINATION ${DATA_INSTALL_DIR}/plasma/services)
install(FILES data/operations/plasmoidservice.operations DESTINATION ${DATA_INSTALL_DIR}/plasma/services)
install(FILES data/operations/storage.operations DESTINATION ${DATA_INSTALL_DIR}/plasma/services)

