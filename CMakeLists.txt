cmake_minimum_required(VERSION 2.8.12)

project(Plasma)

# Make CPack available to easy generate binary packages
include(CPack)
include(FeatureSummary)
include(GenerateExportHeader)

################# set KDE specific information #################

find_package(ECM 0.0.11 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(ECMGenerateHeaders)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

if(KDE_PLATFORM_FEATURE_DISABLE_DEPRECATED)
   set(KDE_NO_DEPRECATED TRUE)
   set(CMAKE_AUTOMOC_MOC_OPTIONS "-DKDE_NO_DEPRECATED")
endif()

############### Load the CTest options ###############
# CTestCustom.cmake has to be in the CTEST_BINARY_DIR.
# in the KDE build system, this is the same as CMAKE_BINARY_DIR.
configure_file(${CMAKE_SOURCE_DIR}/CTestCustom.cmake ${CMAKE_BINARY_DIR}/CTestCustom.cmake COPYONLY)

################# Enable C++0x (still too early for -std=c++11) features for clang and gcc #################

if(UNIX)
   #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++0x")
   add_definitions("-Wall -std=c++0x")
endif()

################# now find all used packages #################

set (QT_MIN_VERSION "5.2.0")
set(KF5_VERSION "4.97.0")

find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Quick Sql Qml Widgets Svg Declarative Script Test)

find_package(KF5Activities ${KF5_VERSION} REQUIRED)
find_package(KF5Archive ${KF5_VERSION} REQUIRED)
find_package(KF5Auth ${KF5_VERSION} REQUIRED)
find_package(KF5Config ${KF5_VERSION} REQUIRED)
find_package(KF5ConfigWidgets ${KF5_VERSION} REQUIRED)
find_package(KF5CoreAddons ${KF5_VERSION} REQUIRED)
find_package(KF5Crash ${KF5_VERSION} REQUIRED)
find_package(KF5DBusAddons ${KF5_VERSION} REQUIRED)
find_package(KF5DNSSD ${KF5_VERSION} REQUIRED)
find_package(KF5Declarative ${KF5_VERSION} REQUIRED)
find_package(KF5GlobalAccel ${KF5_VERSION} REQUIRED)
find_package(KF5GuiAddons ${KF5_VERSION} REQUIRED)
find_package(KF5I18n ${KF5_VERSION} REQUIRED)
find_package(KF5IconThemes ${KF5_VERSION} REQUIRED)
find_package(KF5KIO ${KF5_VERSION} REQUIRED)
find_package(KF5Parts ${KF5_VERSION} REQUIRED)
find_package(KF5Service ${KF5_VERSION} REQUIRED)
find_package(KF5Solid ${KF5_VERSION} REQUIRED)
find_package(KF5ThreadWeaver ${KF5_VERSION} REQUIRED)
find_package(KF5Wallet ${KF5_VERSION} REQUIRED)
find_package(KF5WidgetsAddons ${KF5_VERSION} REQUIRED)
find_package(KF5WindowSystem ${KF5_VERSION} REQUIRED)
find_package(KF5XmlGui ${KF5_VERSION} REQUIRED)

find_package(KF5DocTools ${KF5_VERSION})
set_package_properties(X11 PROPERTIES DESCRIPTION "Tools to generate documentation"
                       TYPE OPTIONAL
                      )

#optional features
find_package(X11 MODULE)
set_package_properties(X11 PROPERTIES DESCRIPTION "X11 libraries"
                       URL "http://www.x.org"
                       TYPE OPTIONAL
                      )
find_package(XCB MODULE COMPONENTS XCB COMPOSITE DAMAGE SHAPE)
set_package_properties(XCB PROPERTIES DESCRIPTION "X protocol C-language Binding"
                       URL "http://xcb.freedesktop.org"
                       TYPE OPTIONAL
                      )
if(X11_FOUND AND XCB_XCB_FOUND)
  set(HAVE_X11 1)
  find_package(Qt5 REQUIRED NO_MODULE COMPONENTS X11Extras)
  #X11_Xrender discovery is done by FindX11
  #add_feature_info("X Rendering Extension (libXrender)" X11_Xrender_FOUND "Support for compositing, rendering operations, and alpha-blending. STRONGLY RECOMMENDED")
endif()

#FIXME: when we have a qca for qt5, reenable
# find_package(QCA2 2.0.0 MODULE)
set_package_properties(QCA2 PROPERTIES DESCRIPTION "Support for remote plasma widgets"
                       URL "http://delta.affinix.com/qca"
                       TYPE OPTIONAL
                      )

#find_package(DBusMenuQt MODULE)
#set_package_properties(DBusMenuQt PROPERTIES DESCRIPTION "Support for notification area menus via the DBusMenu protocol"
#URL "https://launchpad.net/libdbusmenu-qt"
#TYPE REQUIRED
#)



find_package(OpenGL)
set_package_properties(OpenGL PROPERTIES DESCRIPTION "The OpenGL libraries"
                       URL "http://www.opengl.org"
                       TYPE OPTIONAL
                      )

find_package(EGL)
set_package_properties(EGL PROPERTIES
                       PURPOSE "Support for Window Thumbnail on EGL platform"
                       TYPE OPTIONAL
                      )
set(HAVE_EGL ${EGL_FOUND})

if(OPENGL_FOUND AND (${Qt5Gui_OPENGL_IMPLEMENTATION} STREQUAL "GL"))
    set(HAVE_GLX ${HAVE_X11})
else()
    set(HAVE_GLX 0)
endif()

#########################################################################

add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)

#add_definitions(-Wno-deprecated)
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
add_definitions(-DQT_NO_URL_CAST_FROM_STRING)

remove_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_STRICT_ITERATORS -DQT_NO_CAST_FROM_BYTEARRAY -DQT_NO_KEYWORDS)

include(KF5PlasmaMacros.cmake)

################# configure checks and create the configured files #################

# now create config headers
configure_file(config-prefix.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-prefix.h )
configure_file(config-compiler.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-compiler.h )

################ create PlasmaConfig.cmake and install it ###########################

# create a Config.cmake and a ConfigVersion.cmake file and install them
include(ECMPackageConfigHelpers)

include(ECMSetupVersion)
ecm_setup_version(${KF5_VERSION}
                  VARIABLE_PREFIX PLASMA
                  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/plasma_version.h"
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5PlasmaConfigVersion.cmake"
                  SOVERSION 5)

set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5Plasma")

ecm_configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5PlasmaConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5PlasmaConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
  PATH_VARS  INCLUDE_INSTALL_DIR CMAKE_INSTALL_PREFIX
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5PlasmaConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5PlasmaConfigVersion.cmake"
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5PlasmaMacros.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF5PlasmaTargets
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE KF5PlasmaTargets.cmake
        NAMESPACE KF5::
        COMPONENT Devel)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/plasma_version.h
        DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel )

# make plasma_version.h available
include_directories(${CMAKE_CURRENT_BINARY_DIR})

################# list the subdirectories #################
if (KF5DocTools)
    add_subdirectory(doc)
endif()
add_subdirectory(src)
add_subdirectory(desktoptheme)
add_subdirectory(examples)
add_subdirectory(autotests)
add_subdirectory(tests)

feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
