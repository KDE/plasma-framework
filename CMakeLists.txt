cmake_minimum_required(VERSION 2.8.10.20130309)

project(SolidX)

# find_package(ECM 0.0.8 REQUIRED NO_MODULE)

# set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# find_package(KF5 5.0.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

# find_package(Qt5 5.2.0 REQUIRED NO_MODULE COMPONENTS Core Gui)

include(FeatureSummary)
include(CMakePackageConfigHelpers)
# include(ECMSetupVersion)

# ecm_setup_version(
#     5 0 0 VARIABLE_PREFIX SOLIDX
#     VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/solidx_version.h"
#     PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/SolidXConfigVersion.cmake"
#     )

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_BINARY_DIR}/src)

add_subdirectory(src/solidx)

set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/SolidX")

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
