project(PlasmaQuick)

set(plasmaquick_LIB_SRC
    appletquickitem.cpp
    plasmaquickview.cpp
    configmodel.cpp
    configview.cpp
    packageurlinterceptor.cpp
    private/configcategory_p.cpp
)

add_library(KF5PlasmaQuick SHARED ${plasmaquick_LIB_SRC})
add_library(KF5::PlasmaQuick ALIAS KF5PlasmaQuick)

target_link_libraries(KF5PlasmaQuick
    Qt5::Quick
    Qt5::Qml
    KF5::WindowSystem
    KF5::KIOWidgets
    KF5::I18n
    KF5::Service
    KF5::CoreAddons
    KF5::XmlGui
    KF5::Plasma
    KF5::Declarative
)

set_target_properties(KF5PlasmaQuick PROPERTIES
                                     VERSION ${PLASMA_VERSION_STRING}
                                     SOVERSION ${PLASMA_SOVERSION}
                                     EXPORT_NAME PlasmaQuick
                      )

install(TARGETS KF5PlasmaQuick EXPORT KF5PlasmaQuickTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

generate_export_header(KF5PlasmaQuick BASE_NAME PlasmaQuick)

set(plasmaquick_LIB_INCLUDES
    ${CMAKE_CURRENT_BINARY_DIR}/plasmaquick_export.h
    appletquickitem.h
    plasmaquickview.h
    configview.h
    configmodel.h
    packageurlinterceptor.h
)

install(FILES ${plasmaquick_LIB_INCLUDES}
        DESTINATION ${INCLUDE_INSTALL_DIR}/plasmaquick COMPONENT Devel)

install(DIRECTORY
        includes/PlasmaQuick
        DESTINATION ${INCLUDE_INSTALL_DIR}/KDE COMPONENT Devel)

set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5PlasmaQuick")

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KF5PlasmaQuickConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KF5PlasmaQuickConfig.cmake"
    INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
    PATH_VARS  INCLUDE_INSTALL_DIR CMAKE_INSTALL_PREFIX
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/KF5PlasmaQuickConfig.cmake"
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}" COMPONENT Devel
)

install(EXPORT KF5PlasmaQuickTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5PlasmaQuickTargets.cmake NAMESPACE KF5:: )

add_subdirectory(autotests)
