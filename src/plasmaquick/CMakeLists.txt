add_library(KF6PlasmaQuick SHARED)
add_library(KF6::PlasmaQuick ALIAS KF6PlasmaQuick)

set_target_properties(KF6PlasmaQuick PROPERTIES
    VERSION     ${PLASMA_VERSION}
    SOVERSION   ${PLASMA_SOVERSION}
    EXPORT_NAME PlasmaQuick
)

target_sources(KF6PlasmaQuick PRIVATE
    appletquickitem.cpp
    debug_p.cpp
    dialog.cpp
    dialogshadows.cpp
    containmentview.cpp
    configmodel.cpp
    configview.cpp
    packageurlinterceptor.cpp
    private/configcategory_p.cpp
    ../declarativeimports/core/framesvgitem.cpp
    ../declarativeimports/core/managedtexturenode.cpp
    ../declarativeimports/core/units.cpp
)

if(HAVE_KWAYLAND)
    target_sources(KF6PlasmaQuick PRIVATE waylandintegration.cpp)
endif()

ecm_qt_declare_logging_category(KF6PlasmaQuick
    HEADER debug_p.h
    IDENTIFIER LOG_PLASMAQUICK
    CATEGORY_NAME kf.plasma.quick
    OLD_CATEGORY_NAMES org.kde.plasmaquick
    DESCRIPTION "Plasma Quick lib"
    EXPORT PLASMA
)

target_include_directories(KF6PlasmaQuick PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR};${CMAKE_CURRENT_BINARY_DIR}/..>")

target_link_libraries(KF6PlasmaQuick
    PUBLIC
        Qt6::Gui
        Qt6::Quick
        Qt6::Qml
        KF6::Plasma
        KF6::WindowSystem
    PRIVATE
        Qt6::Svg
        KF6::I18n
        KF6::IconThemes
        KF6::CoreAddons
        KF6::XmlGui
        KF6::QuickAddons
        KF6::Package
)

if(HAVE_KWAYLAND)
    target_link_libraries(KF6PlasmaQuick
        PRIVATE
        KF6::WaylandClient
    )
endif()

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
    REQUIRED_HEADERS plasmaquick_LIB_INCLUDES
    PREFIX PlasmaQuick
)

install(FILES ${plasmaquick_LIB_INCLUDES}
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/plasmaquick COMPONENT Devel)

install(FILES ${PlasmaQuick_CamelCase_HEADERS}
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/PlasmaQuick COMPONENT Devel)

set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6PlasmaQuick")

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KF6PlasmaQuickConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KF6PlasmaQuickConfig.cmake"
    INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
    PATH_VARS  KDE_INSTALL_INCLUDEDIR_KF CMAKE_INSTALL_PREFIX
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
