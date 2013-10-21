cmake_minimum_required(VERSION 2.8.11)

project(Plasma)

# Make CPack available to easy generate binary packages
include(CPack)
include(FeatureSummary)

################# set KDE specific information #################

find_package(ECM 0.0.8 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)

#add_definitions(-Wno-deprecated) 

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

find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Quick X11Extras Sql UiTools Qml Widgets Svg Declarative Script Test)

# Load CMake, Compiler and InstallDirs settings from KF5 and the following are already somewhat "done" tier1/tier2 libs from kdelibs:
find_package(KF5 MODULE REQUIRED COMPONENTS CMake Compiler InstallDirs
                                            KIdleTime ItemModels KWidgetsAddons KWindowSystem KCodecs KArchive KCoreAddons Solid ThreadWeaver
                                            KConfig KAuth KJS KWallet KDBusAddons KI18n KGuiAddons KConfigWidgets
                                            KService ItemViews KNotifications KIconThemes KCompletion KJobWidgets KConfigWidgets Sonnet KTextWidgets XmlGui KCrash KDeclarative
                                            KBookmarks KUnitConversion KDE4Attic Kross)
#find_package(KF5Transitional REQUIRED)

# those are not "done" yet:
find_package(KIO REQUIRED NO_MODULE)

# Needed for some includes from KDE/ (KMimeType, etc.)
# FIXME: remove when kdeui is fully split up
find_package(KDELibs4 REQUIRED NO_MODULE)

# must be after KDELibs4, since it links against imported targets coming from KDELibs4Config.cmake:
find_package(KDE4Support REQUIRED NO_MODULE)

#optional features
find_package(X11 MODULE)
find_package(XCB MODULE COMPONENTS XCB COMPOSITE DAMAGE)
set_package_properties(XCB PROPERTIES DESCRIPTION "X protocol C-language Binding"
                       URL "http://xcb.freedesktop.org"
                       TYPE REQUIRED
                      )
if(X11_FOUND AND XCB_XCB_FOUND)
  set(HAVE_X11 1)
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

#find_package(DBusMenuQt MODULE)
#set_package_properties(DBusMenuQt PROPERTIES DESCRIPTION "Support for notification area menus via the DBusMenu protocol"
#URL "https://launchpad.net/libdbusmenu-qt"
#TYPE REQUIRED
#)

# TODO remove it in future
find_package(LibAttica NO_MODULE)
set_package_properties(LibAttica PROPERTIES DESCRIPTION "Support for Get Hot New Stuff related stuff"
                       URL "https://projects.kde.org/attica"
                       TYPE REQUIRED
                      )

find_package(OpenGL)
set_package_properties(OpenGL PROPERTIES DESCRIPTION "The OpenGL libraries"
                       URL "http://www.opengl.org"
                       TYPE OPTIONAL
                      )
if(OPENGL_FOUND)
    set(HAVE_GLX ${HAVE_X11})
else()
    set(HAVE_GLX 0)
endif()

#########################################################################

#add_definitions(${KDE4_DEFINITIONS})
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
add_definitions(-DQT_NO_URL_CAST_FROM_STRING)

remove_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_STRICT_ITERATORS -DQT_NO_CAST_FROM_BYTEARRAY -DQT_NO_KEYWORDS)

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
ecm_setup_version(2.0.0 VARIABLE_PREFIX PLASMA
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
add_subdirectory(desktoptheme)
add_subdirectory(examples)

install(EXPORT PlasmaTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE PlasmaTargets.cmake NAMESPACE KF5:: COMPONENT Devel)
feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
