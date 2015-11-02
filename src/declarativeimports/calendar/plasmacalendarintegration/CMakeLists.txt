project(plasma-calendar-integration)

set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/PlasmaCalendarIntegration")

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(ECMPackageConfigHelpers)
include(ECMSetupVersion)
include(CMakePackageConfigHelpers)
include(WriteBasicConfigVersionFile)
include(KDECMakeSettings)

ecm_setup_version(${PROJECT_VERSION}
  VARIABLE_PREFIX PLASMACALENDARINTEGRATION
  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/plasmacalendarintegration_version.h"
  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/PlasmaCalendarIntegrationConfigVersion.cmake"
  SOVERSION 1)

set(calendar-integration_SRCS
    calendareventsplugin.cpp
    eventdata_p.cpp
)

add_library(plasmacalendarintegration SHARED ${calendar-integration_SRCS})

target_link_libraries(plasmacalendarintegration
    Qt5::Core
)

target_include_directories(plasmacalendarintegration INTERFACE "$<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}/PlasmaCalendarIntegration>")
set_target_properties(plasmacalendarintegration PROPERTIES VERSION ${PROJECT_VERSION}
                                                SOVERSION 1
                                                EXPORT_NAME PlasmaCalendarIntegration
)

ecm_configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/PlasmaCalendarIntegrationConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/PlasmaCalendarIntegrationConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/PlasmaCalendarIntegrationConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/PlasmaCalendarIntegrationConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(TARGETS plasmacalendarintegration EXPORT PlasmaCalendarIntegrationTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(EXPORT PlasmaCalendarIntegrationTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE PlasmaCalendarIntegrationTargets.cmake)

install (FILES
        calendareventsplugin.h
        plasmacalendarintegration_export.h
        ${CMAKE_CURRENT_BINARY_DIR}/plasmacalendarintegration_version.h
        DESTINATION ${INCLUDE_INSTALL_DIR}/PlasmaCalendarIntegration COMPONENT Devel
)
