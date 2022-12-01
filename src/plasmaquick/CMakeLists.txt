add_library(KF5PlasmaQuick SHARED)
add_library(KF5::PlasmaQuick ALIAS KF5PlasmaQuick)

set_target_properties(KF5PlasmaQuick PROPERTIES
    VERSION     ${PLASMA_VERSION}
    SOVERSION   ${PLASMA_SOVERSION}
    EXPORT_NAME PlasmaQuick
)

target_sources(KF5PlasmaQuick PRIVATE
    appletquickitem.cpp
    debug_p.cpp
    dialog.cpp
    dialogshadows.cpp
    view.cpp
    containmentview.cpp
    configmodel.cpp
    shellpluginloader.cpp
    configview.cpp
    packageurlinterceptor.cpp
    private/configcategory_p.cpp
    ../declarativeimports/core/framesvgitem.cpp
    ../declarativeimports/core/managedtexturenode.cpp
    ../declarativeimports/core/units.cpp
)

if(HAVE_KWAYLAND)
    target_sources(KF5PlasmaQuick PRIVATE waylandintegration.cpp)
endif()

ecm_qt_declare_logging_category(KF5PlasmaQuick
    HEADER debug_p.h
    IDENTIFIER LOG_PLASMAQUICK
    CATEGORY_NAME kf.plasma.quick
    OLD_CATEGORY_NAMES org.kde.plasmaquick
    DESCRIPTION "Plasma Quick lib"
    EXPORT PLASMA
)

target_include_directories(KF5PlasmaQuick PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR};${CMAKE_CURRENT_BINARY_DIR}/..>")

target_link_libraries(KF5PlasmaQuick
    PUBLIC
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_MAJOR_VERSION}::Quick
        Qt${QT_MAJOR_VERSION}::Qml
        KF5::Plasma
        KF5::WindowSystem
    PRIVATE
        Qt${QT_MAJOR_VERSION}::Svg
        KF5::I18n
        KF5::IconThemes
        KF5::Service
        KF5::CoreAddons
        KF5::XmlGui
        KF5::Declarative
        KF5::QuickAddons
)

if(HAVE_KWAYLAND)
    target_link_libraries(KF5PlasmaQuick
        PRIVATE
        KF5::WaylandClient
    )
endif()

if(HAVE_X11)
    target_link_libraries(KF5PlasmaQuick
        PRIVATE
            ${X11_LIBRARIES}
            XCB::XCB
    )
    if (TARGET Qt5::X11Extras)
        target_link_libraries(KF5PlasmaQuick PRIVATE Qt5::X11Extras)
    elseif(TARGET Qt6::Gui)
        target_link_libraries(KF5PlasmaQuick PRIVATE Qt6::GuiPrivate)
    endif()

    if(HAVE_XCB_SHAPE)
        target_link_libraries(KF5PlasmaQuick PRIVATE XCB::SHAPE)
    endif()
endif()

install(TARGETS KF5PlasmaQuick EXPORT KF5PlasmaQuickTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

ecm_generate_export_header(KF5PlasmaQuick
    BASE_NAME PlasmaQuick
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    DEPRECATED_BASE_VERSION 0
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
    DEPRECATION_VERSIONS 5.12 5.25 5.36
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

set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5PlasmaQuick")

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KF5PlasmaQuickConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KF5PlasmaQuickConfig.cmake"
    INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
    PATH_VARS  KDE_INSTALL_INCLUDEDIR_KF CMAKE_INSTALL_PREFIX
)

ecm_setup_version(${KF_VERSION}
                  VARIABLE_PREFIX PLASMAQUICK
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5PlasmaQuickConfigVersion.cmake" )

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/KF5PlasmaQuickConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/KF5PlasmaQuickConfigVersion.cmake"
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}" COMPONENT Devel
)

install(EXPORT KF5PlasmaQuickTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5PlasmaQuickTargets.cmake NAMESPACE KF5:: )
