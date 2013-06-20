
cmake_minimum_required(VERSION 2.8.10.1)

project(plasma)

################# Disallow in-source build #################

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
   message(FATAL_ERROR "plasma requires an out of source build. Please create a separate build directory and run 'cmake path_to_plasma [options]' there.")
endif()


# Make CPack available to easy generate binary packages
include(CPack)

include(FeatureSummary)

################# set KDE specific information #################

find_package(ECM 0.0.8 REQUIRED NO_MODULE)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

find_package(Qt5Core REQUIRED NO_MODULE)

find_package(Qt5Transitional MODULE)
find_package(Qt5Quick REQUIRED NO_MODULE)
find_package(Qt5X11Extras REQUIRED NO_MODULE)
find_package(kde4support REQUIRED NO_MODULE)

add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)

if(KDE_PLATFORM_FEATURE_DISABLE_DEPRECATED)
   set(KDE_NO_DEPRECATED TRUE)
   set(CMAKE_AUTOMOC_MOC_OPTIONS "-DKDE_NO_DEPRECATED")
endif()

############### Load the CTest options ###############
# CTestCustom.cmake has to be in the CTEST_BINARY_DIR.
# in the KDE build system, this is the same as CMAKE_BINARY_DIR.
configure_file(${CMAKE_SOURCE_DIR}/CTestCustom.cmake ${CMAKE_BINARY_DIR}/CTestCustom.cmake COPYONLY)

################# now find all used packages #################

set (QT_MIN_VERSION "5.2.0")

find_package(kdeqt5staging REQUIRED NO_MODULE)

# Load CMake, Compiler and InstallDirs settings from KF5 and the following are already somewhat "done" tier1/tier2 libs from kdelibs:
find_package(KF5 MODULE REQUIRED COMPONENTS CMake Compiler InstallDirs
                                            XmlGui KIdleTime ItemModels KWidgetsAddons KWindowSystem KCodecs KArchive KCoreAddons Solid ThreadWeaver
                                            KConfig KAuth kjs
                                            ki18n kguiaddons kservice kwidgets ItemViews KNotifications KCompletion KJobWidgets KConfigWidgets KIconThemes)
#find_package(KF5Transitional REQUIRED)

# those are not "done" yet:
find_package(ki18n REQUIRED NO_MODULE)
find_package(kio REQUIRED NO_MODULE)
find_package(kdeclarative REQUIRED NO_MODULE)

# Needed for some includes from KDE/ (KMimeType, etc.)
# FIXME: remove when kdeui is fully split up
find_package(KDELibs4 REQUIRED NO_MODULE)

#optional features
find_package(X11 MODULE)
if(X11_FOUND)
  set(HAVE_X11 X11_FOUND)
  #X11_Xrender discovery is done by FindX11
  add_feature_info("X Rendering Extension (libXrender)" X11_Xrender_FOUND "Support for compositing, rendering operations, and alpha-blending. STRONGLY RECOMMENDED")
  add_feature_info("X Screensaver Extension (libXss)" X11_Xscreensaver_FOUND "Support for KIdleTime (fallback mode)")
  add_feature_info("X Sync Extension (libXext)" X11_XSync_FOUND "Efficient operation of KIdleTime. STRONGLY RECOMMENDED")

  if(NOT X11_XSync_FOUND AND NOT X11_Xscreensaver_FOUND)
    message(FATAL_ERROR "\nNeither the XSync (libXext) nor XScreensaver (libXss) development package was found.\nPlease install one of them (XSync is recommended)\n")
  endif()

  #X11 Session Management (SM) is required
  #X11_SM_FOUND is set in FindX11, which is required by KDE4Internal
  if(UNIX AND NOT X11_SM_FOUND)
    message(FATAL_ERROR "\nThe X11 Session Management (SM) development package could not be found.\nPlease install libSM.\n")
  endif()
endif()

find_package(OpenSSL MODULE)
set_package_properties(OpenSSL PROPERTIES DESCRIPTION "Support for secure network communications (SSL and TLS)"
                       URL "http://openssl.org"
                       TYPE RECOMMENDED
                       PURPOSE "KDE uses OpenSSL for the bulk of secure communications, including secure web browsing via HTTPS"
                      )

find_package(Libintl MODULE)
set_package_properties(Libintl PROPERTIES DESCRIPTION "Support for multiple languages"
                       URL "http://www.gnu.org/software/gettext"
                       TYPE RECOMMENDED
                       PURPOSE "Enables KDE to be available in many different languages"
                      )

#FIXME: when we have a qca for qt5, reenable
find_package(QCA2 2.0.0 MODULE)
set_package_properties(QCA2 PROPERTIES DESCRIPTION "Support for remote plasma widgets"
                       URL "http://delta.affinix.com/qca"
                       TYPE OPTIONAL
                      )

find_package(DBusMenuQt MODULE)
set_package_properties(DBusMenuQt PROPERTIES DESCRIPTION "Support for notification area menus via the DBusMenu protocol"
                       URL "https://launchpad.net/libdbusmenu-qt"
                       TYPE REQUIRED
                      )

#########################################################################

#add_definitions(${KDE4_DEFINITIONS})
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
add_definitions(-DQT_NO_URL_CAST_FROM_STRING)

remove_definitions(-DQT3_SUPPORT_WARNINGS -DQT3_SUPPORT)
remove_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_STRICT_ITERATORS -DQT_NO_CAST_FROM_BYTEARRAY -DQT_NO_KEYWORDS)


################# setup the include directories #################
include_directories( ${kdeqt5staging_INCLUDE_DIRS}
                     ${kdeqt5staging_INCLUDE_DIR}
                     ${KDE4_INCLUDES}
                     ${KF5_INCLUDE_DIRS}  # since e-c-m 0.0.7
                   )

# for including config.h and for includes like <kparts/foo.h>
include_directories( ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src ${CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR}/src ${CMAKE_SOURCE_DIR}/interfaces ${CMAKE_SOURCE_DIR}/src/plasma/includes)

if(QCA2_FOUND)
    include_directories(
        ${QCA2_INCLUDE_DIR}
    )
endif()

include(PlasmaMacros.cmake)

################# configure checks and create the configured files #################

# now create config headers
configure_file(config-prefix.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-prefix.h )
configure_file(config-compiler.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-compiler.h )

################ create PlasmaConfig.cmake and install it ###########################


# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/Plasma")

include(CMakePackageConfigHelpers)

include(ECMSetupVersion)
ecm_setup_version(2 0 0 VARIABLE_PREFIX PLASMA
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/plasma_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/PlasmaConfigVersion.cmake")

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/PlasmaConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/PlasmaConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
  PATH_VARS  INCLUDE_INSTALL_DIR CMAKE_INSTALL_PREFIX
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/plasma_version.h
        DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel )

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/PlasmaConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/PlasmaConfigVersion.cmake"
  "${CMAKE_CURRENT_SOURCE_DIR}/PlasmaMacros.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

################# list the subdirectories #################
add_subdirectory(src)
add_subdirectory( desktoptheme )

# install(EXPORT plasmaTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
#         FILE plasmaTargets.cmake COMPONENT Devel)
install(EXPORT PlasmaTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE PlasmaTargets.cmake NAMESPACE KF5:: COMPONENT Devel)
feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
