project(PlasmaQuick)

set(plasmaquick_LIB_SRC
    plasmaquickview.cpp
    configmodel.cpp
    configview.cpp
    private/configcategory_p.cpp
    #private/containmentconfigview_p.cpp
    #private/currentcontainmentactionsmodel_p.cpp
)

add_library(PlasmaQuick SHARED ${plasmaquick_LIB_SRC})

target_link_libraries(PlasmaQuick
    ${Qt5Quick_LIBRARIES}
    ${Qt5Qml_LIBRARIES}
    ${KWindowSystem_LIBRARIES}
    KF5::KI18n
    KF5::KService
    KF5::KCoreAddons
    ${XmlGui_LIBRARIES}
    plasma
    KF5::KDeclarative
)

set_target_properties(PlasmaQuick PROPERTIES
                             VERSION 5.0.0
                             SOVERSION 5
                      )

install(TARGETS PlasmaQuick EXPORT PlasmaQuickTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

generate_export_header(PlasmaQuick)

set(plasmaquick_LIB_INCLUDES
    ${CMAKE_CURRENT_BINARY_DIR}/plasmaquick_export.h
    plasmaquickview.h
    configview.h
    configmodel.h
)

install(FILES ${plasmaquick_LIB_INCLUDES}
        DESTINATION ${INCLUDE_INSTALL_DIR}/plasmaquick COMPONENT Devel)

install(DIRECTORY
        includes/PlasmaQuick
        DESTINATION ${INCLUDE_INSTALL_DIR}/KDE COMPONENT Devel)

set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/PlasmaQuick")

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/PlasmaQuickConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/PlasmaQuickConfig.cmake"
    INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
    PATH_VARS  INCLUDE_INSTALL_DIR CMAKE_INSTALL_PREFIX
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/PlasmaQuickConfig.cmake"
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}" COMPONENT Devel
)

install(EXPORT PlasmaQuickTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE PlasmaQuickTargets.cmake NAMESPACE KF5:: )

