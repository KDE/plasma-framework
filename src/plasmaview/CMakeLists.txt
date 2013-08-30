project(PlasmaView)

set(plasmaview_LIB_SRC
    view.cpp
    configview.cpp
    containmentconfigview.cpp
    currentcontainmentactionsmodel_p.cpp
)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    )

add_library(plasmaview SHARED ${plasmaview_LIB_SRC})

target_link_libraries(plasmaview
    ${Qt5Quick_LIBRARIES}
    ${Qt5Qml_LIBRARIES}
    ${KWindowSystem_LIBRARIES}
    plasma
    kdeclarative
    KF5::KI18n
    KF5::KService
    KF5::KCoreAddons
    KF5::XmlGui
)

set_target_properties(plasmaview PROPERTIES
                             VERSION 5.0.0
                             SOVERSION 5
                      )

install(TARGETS plasmaview EXPORT PlasmaViewTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

generate_export_header(plasmaview)

set(plasmaview_LIB_INCLUDES
    ${CMAKE_CURRENT_BINARY_DIR}/plasmaview_export.h
    view.h
    configview.h
)

install(FILES ${plasmaview_LIB_INCLUDES}
        DESTINATION ${INCLUDE_INSTALL_DIR}/plasmaview COMPONENT Devel)

install(DIRECTORY
        includes/PlasmaView
        DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/ COMPONENT Devel)

set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/PlasmaView")

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/PlasmaViewConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/PlasmaViewConfig.cmake"
    INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
    PATH_VARS  INCLUDE_INSTALL_DIR CMAKE_INSTALL_PREFIX
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/PlasmaViewConfig.cmake"
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}" COMPONENT Devel
)

install(EXPORT PlasmaViewTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE PlasmaViewTargets.cmake NAMESPACE KF5:: )

