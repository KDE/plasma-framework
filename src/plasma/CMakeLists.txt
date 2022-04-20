add_subdirectory(packagestructure)

if(KDE_PLATFORM_FEATURE_BINARY_COMPATIBLE_FEATURE_REDUCTION)
    set(PLASMA_NO_PACKAGE_EXTRADATA TRUE)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-plasma.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-plasma.h)

add_library(KF5Plasma)
add_library(KF5::Plasma ALIAS KF5Plasma)

set_target_properties(KF5Plasma PROPERTIES
    VERSION     ${PLASMA_VERSION}
    SOVERSION   ${PLASMA_SOVERSION}
    EXPORT_NAME Plasma
)

target_sources(KF5Plasma PRIVATE
#global
    plasma.cpp
    pluginloader.cpp
    version.cpp

#applets,containments,corona
    applet.cpp
    containment.cpp
    containmentactions.cpp
    corona.cpp
    private/applet_p.cpp
    private/associatedapplicationmanager.cpp
    private/containment_p.cpp
    private/timetracker.cpp

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

#graphics
    framesvg.cpp
    svg.cpp
    theme.cpp
    private/theme_p.cpp

#scripting
    scripting/appletscript.cpp
    scripting/dataenginescript.cpp
    scripting/scriptengine.cpp
)

if(NOT EXCLUDE_DEPRECATED_BEFORE_AND_AT STREQUAL "CURRENT" AND
   EXCLUDE_DEPRECATED_BEFORE_AND_AT VERSION_LESS 5.83.0)
    #packages
    target_sources(KF5Plasma PRIVATE package.cpp packagestructure.cpp)
endif()


if(HAVE_X11)
    target_sources(KF5Plasma PRIVATE private/effectwatcher.cpp)
endif()

kconfig_add_kcfg_files(KF5Plasma data/kconfigxt/libplasma-theme-global.kcfgc)

ecm_qt_declare_logging_category(KF5Plasma
    HEADER debug_p.h
    IDENTIFIER LOG_PLASMA
    CATEGORY_NAME kf.plasma.core
    OLD_CATEGORY_NAMES org.kde.plasma
    DESCRIPTION "Plasma Core lib"
    EXPORT PLASMA
)

ecm_generate_export_header(KF5Plasma
    EXPORT_FILE_NAME plasma/plasma_export.h
    BASE_NAME Plasma
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    DEPRECATED_BASE_VERSION 0
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
    DEPRECATION_VERSIONS 5.6 5.19 5.28 5.30 5.36 5.46 5.67 5.77 5.78 5.81 5.83 5.85 5.86 5.88 5.94
)

if(HAVE_X11)
    set(PLASMA_EXTRA_LIBS ${PLASMA_EXTRA_LIBS} XCB::XCB)
    if (TARGET Qt5::X11Extras)
        list(APPEND PLASMA_EXTRA_LIBS Qt5::X11Extras)
    elseif(TARGET Qt6::Gui)
        list(APPEND PLASMA_EXTRA_LIBS Qt6::GuiPrivate)
    endif()
endif()

target_link_libraries(KF5Plasma
PUBLIC
    Qt${QT_MAJOR_VERSION}::Gui
    KF5::ConfigCore
    KF5::CoreAddons
PRIVATE
    Qt${QT_MAJOR_VERSION}::Sql
    Qt${QT_MAJOR_VERSION}::Svg
    Qt${QT_MAJOR_VERSION}::DBus
    KF5::Archive
    KF5::GuiAddons #kimagecache
    KF5::I18n
    KF5::KIOCore    #ServiceJob
    KF5::KIOWidgets  #KIO::JobUiDelegate
    KF5::WindowSystem #compositingActive
    KF5::Declarative #runtimePlatform
    KF5::XmlGui   #KActionCollection
    KF5::GlobalAccel #Applet::setGlobalShortcut
    KF5::Notifications
    KF5::IconThemes
    ${PLASMA_EXTRA_LIBS}
)

if(NOT EXCLUDE_DEPRECATED_BEFORE_AND_AT STREQUAL "CURRENT" AND
   EXCLUDE_DEPRECATED_BEFORE_AND_AT VERSION_LESS 5.94.0)
    target_link_libraries(KF5Plasma PUBLIC KF5::Service) # For kplugininfo.h and kservice.h
endif()
if(NOT EXCLUDE_DEPRECATED_BEFORE_AND_AT STREQUAL "CURRENT" AND
   EXCLUDE_DEPRECATED_BEFORE_AND_AT VERSION_LESS 5.100.0)
    target_link_libraries(KF5Plasma PUBLIC KF5::Package)
else()
    target_link_libraries(KF5Plasma PRIVATE KF5::Package)
endif()

set(Plasma_BUILD_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/Plasma
)
target_include_directories(KF5Plasma PUBLIC
  "$<BUILD_INTERFACE:${Plasma_BUILD_INCLUDE_DIRS}>"
)
target_include_directories(KF5Plasma INTERFACE
  "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF};${KDE_INSTALL_INCLUDEDIR_KF}/Plasma>"
)

########### install files ###############
ecm_generate_headers(Plasma_CamelCase_HEADERS
    HEADER_NAMES
        Applet
        Containment
        ContainmentActions
        Corona
        DataContainer
        DataEngine
        DataEngineConsumer
        PluginLoader
        FrameSvg
        Package
        PackageStructure
        Service
        ServiceJob
        Svg
        Theme
        Plasma
    REQUIRED_HEADERS Plasma_HEADERS
    PREFIX Plasma
)

set(Plasma_HEADERS
    ${Plasma_HEADERS}
    version.h
)

set(PlasmaScripting_HEADERS
    scripting/appletscript.h
    scripting/dataenginescript.h
    scripting/scriptengine.h
)


install(FILES
        ${Plasma_HEADERS}
        ${CMAKE_CURRENT_BINARY_DIR}/plasma/plasma_export.h
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/plasma COMPONENT Devel)

install(FILES ${Plasma_CamelCase_HEADERS}
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/Plasma COMPONENT Devel)


install(FILES ${PlasmaScripting_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/plasma/scripting COMPONENT Devel)

install(FILES
   data/servicetypes/plasma-applet.desktop
   data/servicetypes/plasma-containment.desktop
   data/servicetypes/plasma-containmentactions.desktop
   data/servicetypes/plasma-dataengine.desktop
   data/servicetypes/plasma-generic.desktop
   data/servicetypes/plasma-packagestructure.desktop
   data/servicetypes/plasma-scriptengine.desktop
   data/servicetypes/plasma-service.desktop
   data/servicetypes/plasma-shell.desktop
   data/servicetypes/plasma-lookandfeel.desktop
   DESTINATION ${KDE_INSTALL_KSERVICETYPESDIR})

install(FILES data/operations/dataengineservice.operations DESTINATION ${PLASMA_DATA_INSTALL_DIR}/services)
install(FILES data/operations/plasmoidservice.operations DESTINATION ${PLASMA_DATA_INSTALL_DIR}/services)
install(FILES data/operations/storage.operations DESTINATION ${PLASMA_DATA_INSTALL_DIR}/services)

install(TARGETS KF5Plasma EXPORT KF5PlasmaTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

if(BUILD_QCH)
    ecm_add_qch(
        KF5Plasma_QCH
        NAME Plasma
        BASE_NAME KF5Plasma
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${Plasma_HEADERS}
            ${PlasmaScripting_HEADERS}
            Mainpage.dox
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt5Gui_QCH
            KF5Service_QCH
            KF5Package_QCH
        INCLUDE_DIRS
            ${Plasma_BUILD_INCLUDE_DIRS}
        BLANK_MACROS
            PLASMA_EXPORT
            PLASMA_DEPRECATED
            PLASMA_DEPRECATED_EXPORT
            "PLASMA_DEPRECATED_VERSION(x, y, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()
