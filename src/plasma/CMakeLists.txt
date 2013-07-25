
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
#set_package_properties(KDEPIMLIBS PROPERTIES DESCRIPTION "KDE PIM libraries"
#                       URL "http://www.kde.org" TYPE OPTIONAL
#                       PURPOSE "Needed for building several Plasma DataEngines")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${KDE4_INCLUDE_DIR}
                    ${KDE4_INCLUDE_DIR}/KDE
                    ${KDE4_INCLUDE_DIR}/kio
                    #FIXME: this will go into Qt upstream
                    ${CMAKE_INSTALL_PREFIX}/include/kdeqt5staging/
                    )



if(NOT PLASMA_NO_KIO)
   include_directories(${KDE4_KIO_INCLUDES})
   set(PLASMA_EXTRA_LIBS ${PLASMA_EXTRA_LIBS} ${KDE4_KIO_LIBS})
endif(NOT PLASMA_NO_KIO)

if(NOT PLASMA_NO_SOLID)
    set(PLASMA_EXTRA_LIBS ${PLASMA_EXTRA_LIBS} ${Solid_LIBRARIES})
endif(NOT PLASMA_NO_SOLID)

if(NOT PLASMA_NO_PACKAGEKIT)
    add_definitions(-DPLASMA_ENABLE_PACKAGEKIT_SUPPORT=1)
    set(PLASMA_EXTRA_LIBS ${PLASMA_EXTRA_LIBS} ${QT_QTDBUS_LIBRARY})
endif(NOT PLASMA_NO_PACKAGEKIT)

if (NOT PLASMA_NO_KUTILS)
   #FIXME: for kcmodule classes; should not be in libplasma if possible
    set(PLASMA_EXTRA_LIBS ${PLASMA_EXTRA_LIBS} ${KDE4_KUTILS_LIBS})
endif(NOT PLASMA_NO_KUTILS)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-plasma.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-plasma.h)

#FIXME: gpgme++ is in kdepimlibs, must move somewhere else!
include_directories(${KDEPIMLIBS_INCLUDE_DIRS} ${GPGME_INCLUDES})

add_subdirectory(autotests)
add_definitions(-DKDE_DEFAULT_DEBUG_AREA=1209)

########### next target ###############

set(plasma_LIB_SRCS
#global
    plasma.cpp
    pluginloader.cpp
    version.cpp
    private/componentinstaller.cpp

#runners
    abstractrunner.cpp
    querymatch.cpp
    runnercontext.cpp
    runnermanager.cpp
    runnersyntax.cpp
    private/runnerjobs.cpp

#applets,containments,corona
    applet.cpp
    configloader.cpp
    containment.cpp
    containmentactions.cpp
    corona.cpp
    private/applet_p.cpp
    private/associatedapplicationmanager.cpp
    private/containment_p.cpp

#Dataengines, services
    datacontainer.cpp
    dataengine.cpp
    dataengineconsumer.cpp
    service.cpp
    servicejob.cpp
    private/datacontainer_p.cpp
    private/dataenginemanager.cpp
    private/storage.cpp
    private/storagethread.cpp

#packages
    package.cpp
    packagestructure.cpp
    private/packagejob.cpp
    private/packagejobthread.cpp
    private/packages.cpp

#graphics
    framesvg.cpp
    svg.cpp
    theme.cpp
    private/theme_p.cpp

#scripting
    scripting/appletscript.cpp
    scripting/dataenginescript.cpp
    scripting/runnerscript.cpp
    scripting/scriptengine.cpp
)

set(plasma_LIB_SRCS ${plasma_LIB_SRCS} private/effectwatcher.cpp)

kconfig_add_kcfg_files(plasma_LIB_SRCS data/kconfigxt/libplasma-theme-global.kcfgc)

#NEPOMUK_GENERATE_FROM_ONTOLOGY(
#   nwc.nrl
#   ${metadata_test_BINARY_DIR}
#   TEST_HEADERS
#   TEST_SOURCES
#   TEST_INCLUDES
#)


kde4_add_library(plasma SHARED ${plasma_LIB_SRCS})

target_link_libraries(plasma
    ${QT_QTUITOOLS_LIBRARY}
    ${QT_QTNETWORK_LIBRARY} ${QT_QTSQL_LIBRARY}
    ${Qt5Quick_LIBRARIES}
    ${KDE4_KDEUI_LIBS}
    ${ThreadWeaver_LIBRARIES}
    ${KAuth_LIBRARIES}
    ${KWindowSystem_LIBRARIES}
    ${PLASMA_EXTRA_LIBS}
    KF5::KI18n
    ${KDE4Support_LIBRARIES}
    KF5::KDE4Attic
)
#FIXME gpgme++ is in kdepimlibs, neeeds to be elsewhere
target_link_libraries(plasma kdeclarative ${KArchive_LIBRARIES})

if(X11_FOUND)
   target_link_libraries(plasma ${X11_LIBRARIES})
endif(X11_FOUND)

if(DL_LIBRARY)
    target_link_libraries(plasma ${DL_LIBRARY})
endif(DL_LIBRARY)

target_link_libraries(plasma LINK_INTERFACE_LIBRARIES ${KDE4_KDECORE_LIBS} Qt5::Widgets)

#do NOT use GENERIC versioning -- the plasma team will take care of versioning
set_target_properties(plasma PROPERTIES
                             VERSION 5.0.0
                             SOVERSION 5
                      )

install(TARGETS plasma EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############
generate_export_header(plasma)

set(plasma_LIB_INCLUDES
    abstractrunner.h
    applet.h
    configloader.h
    containment.h
    containmentactions.h
    corona.h
    datacontainer.h
    dataengine.h
    pluginloader.h
    framesvg.h
    package.h
    packagestructure.h
    plasma.h
    ${CMAKE_CURRENT_BINARY_DIR}/plasma_export.h
    querymatch.h
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

install(DIRECTORY
        includes/Plasma
        DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/ COMPONENT Devel)


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
   data/servicetypes/plasma-generic.desktop
   data/servicetypes/plasma-packagestructure.desktop
   data/servicetypes/plasma-runner.desktop
   data/servicetypes/plasma-scriptengine.desktop
   data/servicetypes/plasma-service.desktop
   data/servicetypes/plasma-shell.desktop
   data/servicetypes/plasma-lookandfeel.desktop
   DESTINATION ${SERVICETYPES_INSTALL_DIR})

install(FILES data/operations/dataengineservice.operations DESTINATION ${DATA_INSTALL_DIR}/plasma/services)
install(FILES data/operations/plasmoidservice.operations DESTINATION ${DATA_INSTALL_DIR}/plasma/services)
install(FILES data/operations/storage.operations DESTINATION ${DATA_INSTALL_DIR}/plasma/services)

install(TARGETS plasma EXPORT PlasmaTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

