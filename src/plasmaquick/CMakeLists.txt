# Consumer's include dir which has to be explicitly used to make headers of this lib visible to documented includes
# Results in duplicate of prefix-dir & C++ namespace below, but part of different things, so by design:
# <visibility-layer>/<C++ namespace>/class header files
set(PLASMAQUICK_INSTALL_INCLUDEDIR "${KDE_INSTALL_INCLUDEDIR_KF}/PlasmaQuick")

add_library(KF6PlasmaQuick SHARED)
add_library(KF6::PlasmaQuick ALIAS KF6PlasmaQuick)

set_target_properties(KF6PlasmaQuick PROPERTIES
    VERSION     ${PLASMA_VERSION}
    SOVERSION   ${PLASMA_SOVERSION}
    EXPORT_NAME PlasmaQuick
)

qt_add_resources(RESOURCES private/resources.qrc)

target_sources(KF6PlasmaQuick PRIVATE
    appletcontext.cpp
    appletquickitem.cpp
    appletpopup.cpp
    debug_p.cpp
    dialog.cpp
    dialogshadows.cpp
    containmentview.cpp
    configmodel.cpp
    configview.cpp
    edgeeventforwarder.cpp
    packageurlinterceptor.cpp
    sharedqmlengine.cpp
    quickviewsharedengine.cpp
    plasmawindow.cpp
    popupplasmawindow.cpp
    transientplacementhint.cpp
    windowresizehandler.cpp
    private/configcategory_p.cpp
    private/plasmoidattached_p.cpp
    private/dialogbackground_p.cpp
    plasmoid/plasmoiditem.cpp
    plasmoid/containmentitem.cpp
    plasmoid/dropmenu.cpp
    plasmoid/wallpaperitem.cpp
    waylandintegration.cpp
    ${RESOURCES}
)

qt_generate_wayland_protocol_client_sources(KF6PlasmaQuick
    FILES
        "${PLASMA_WAYLAND_PROTOCOLS_DIR}/plasma-shell.xml"
        "${Wayland_DATADIR}/wayland.xml"
)

ecm_qt_declare_logging_category(KF6PlasmaQuick
    HEADER debug_p.h
    IDENTIFIER LOG_PLASMAQUICK
    CATEGORY_NAME kf.plasma.quick
    OLD_CATEGORY_NAMES org.kde.plasmaquick
    DESCRIPTION "Plasma Quick lib"
    EXPORT PLASMA
)

set_target_properties(KF6PlasmaQuick PROPERTIES
    VERSION     ${PLASMA_VERSION}
    SOVERSION   ${PLASMA_SOVERSION}
    EXPORT_NAME PlasmaQuick
)

target_include_directories(KF6PlasmaQuick
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR};${CMAKE_CURRENT_BINARY_DIR}/..>"
    INTERFACE
        "$<INSTALL_INTERFACE:${PLASMAQUICK_INSTALL_INCLUDEDIR}>"
        "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/Plasma>" # module version header
)

target_link_libraries(KF6PlasmaQuick
    PUBLIC
        Qt6::Gui
        Qt6::Quick
        Qt6::Qml
        KF6::Plasma
        KF6::WindowSystem
    PRIVATE
        Qt6::Svg
        Qt6::GuiPrivate
        Qt6::WaylandClient
        Wayland::Client
        KF6::ConfigGui
        KF6::ConfigQml
        KF6::I18n
        KF6::IconThemes
        KF6::CoreAddons
        KF6::Package
        KF6::KIOCore
        KF6::KIOWidgets
        KF6::Notifications
        KF6::Activities
        KF6::KCMUtilsQuick
        KF6::Svg
)

if(HAVE_X11)
    target_link_libraries(KF6PlasmaQuick
        PRIVATE
            ${X11_LIBRARIES}
            XCB::XCB
            Qt6::GuiPrivate
    )

    if(HAVE_XCB_SHAPE)
        target_link_libraries(KF6PlasmaQuick PRIVATE XCB::SHAPE)
    endif()
endif()

install(TARGETS KF6PlasmaQuick EXPORT KF6PlasmaQuickTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

ecm_generate_export_header(KF6PlasmaQuick
    BASE_NAME PlasmaQuick
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    VERSION_BASE_NAME Plasma
    DEPRECATED_BASE_VERSION 0
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
    DEPRECATION_VERSIONS
)

set(plasmaquick_LIB_INCLUDES
    ${CMAKE_CURRENT_BINARY_DIR}/plasmaquick_export.h
    packageurlinterceptor.h
)

ecm_generate_headers(PlasmaQuick_CamelCase_HEADERS
    HEADER_NAMES
        AppletQuickItem
        ContainmentView
        ConfigView
        ConfigModel
        Dialog
        PlasmaWindow
        SharedQmlEngine
        QuickViewSharedEngine
    REQUIRED_HEADERS plasmaquick_LIB_INCLUDES
    PREFIX PlasmaQuick
)

install(
    FILES ${plasmaquick_LIB_INCLUDES}
    DESTINATION ${PLASMAQUICK_INSTALL_INCLUDEDIR}/plasmaquick
    COMPONENT Devel
)

install(
    FILES ${PlasmaQuick_CamelCase_HEADERS}
    DESTINATION ${PLASMAQUICK_INSTALL_INCLUDEDIR}/PlasmaQuick
    COMPONENT Devel
)

set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6PlasmaQuick")

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KF6PlasmaQuickConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KF6PlasmaQuickConfig.cmake"
    INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
    PATH_VARS  PLASMAQUICK_INSTALL_INCLUDEDIR CMAKE_INSTALL_PREFIX
)

ecm_setup_version(${KF_VERSION}
                  VARIABLE_PREFIX PLASMAQUICK
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6PlasmaQuickConfigVersion.cmake" )

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/KF6PlasmaQuickConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/KF6PlasmaQuickConfigVersion.cmake"
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}" COMPONENT Devel
)

install(EXPORT KF6PlasmaQuickTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF6PlasmaQuickTargets.cmake NAMESPACE KF6:: )
