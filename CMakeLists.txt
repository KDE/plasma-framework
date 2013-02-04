
cmake_minimum_required(VERSION 2.8.10.1)

project(plasma)

# Make CPack available to easy generate binary packages
include(CPack)

################# set KDE specific information #################

find_package(ECM 0.0.6 REQUIRED NO_MODULE)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_INSTALL_PREFIX}/share/cmake/modules" "${CMAKE_SOURCE_DIR}/cmake/modules" ${ECM_MODULE_PATH})
find_path(KDE_MODULES_DIR NAMES KDE4Macros.cmake PATH_SUFFIXES share/cmake/modules ${CMAKE_PREFIX_PATH})

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

find_package(Qt5Core REQUIRED NO_MODULE)

find_package(Qt5Transitional MODULE)
find_package(Qt5Quick REQUIRED)

if (Qt5_POSITION_INDEPENDENT_CODE)
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

# The Qt5Widgets_INCLUDES also includes the include directories for
# dependencies QtCore and QtGui
include_directories(${Qt5Widgets_DEFINITIONS} ${Qt5Quick_DEFINITIONS} ${QT_INCLUDES} ${KDE4_INCLUDES})

# We need add -DQT_WIDGETS_LIB when using QtWidgets in Qt 5.
add_definitions(${Qt5Widgets_DEFINITIONS} ${Qt5Quick_DEFINITIONS} ${Qt5Qml_DEFINITIONS})

set(CMAKE_AUTOMOC ON)


# By default don't add any linked libraries to the "exported"
# link interfaces of shared libraries, so that executables linking
# against these libraries will not automatically add implicit
# dependencies to their link list.
#
# This reduces inter-package dependencies and makes it easier to remove
# dependencies of shared libraries without breaking binary compatibility.
set(CMAKE_LINK_INTERFACE_LIBRARIES "")

if(KDE_PLATFORM_FEATURE_DISABLE_DEPRECATED)
   set(KDE_NO_DEPRECATED TRUE)
   set(CMAKE_AUTOMOC_MOC_OPTIONS "-DKDE_NO_DEPRECATED")
endif(KDE_PLATFORM_FEATURE_DISABLE_DEPRECATED)

############### Load the CTest options ###############
# CTestCustom.cmake has to be in the CTEST_BINARY_DIR.
# in the KDE build system, this is the same as CMAKE_BINARY_DIR.
configure_file(${CMAKE_SOURCE_DIR}/CTestCustom.cmake ${CMAKE_BINARY_DIR}/CTestCustom.cmake COPYONLY)

################# now find all used packages #################

set (QT_MIN_VERSION "5.0.0")

find_package(KDE4Internal REQUIRED)
#find_package(KF5Transitional REQUIRED)
include(KDE4Defaults)
#include (MacroLibrary)

find_package(kidletime REQUIRED)
find_package(itemmodels REQUIRED)
find_package(kwidgetsaddons REQUIRED)
find_package(kwindowsystem REQUIRED)
find_package(kcodecs REQUIRED)
find_package(ki18n REQUIRED)
find_package(kcoreaddons REQUIRED)
find_package(solid REQUIRED)
if (APPLE)
   find_package(Carbon REQUIRED)
endif (APPLE)

#required features:
# Perl is used e.g. in khtml, kjs, kjsembed and others
find_package(Perl)
set_package_properties(Perl PROPERTIES URL "http://www.perl.org" TYPE REQUIRED PURPOSE "Needed for building kdelibs")

find_package(ZLIB)
set_package_properties(ZLIB PROPERTIES DESCRIPTION "Support for gzip compressed files and data streams"
                       URL "http://www.zlib.net"
                       TYPE REQUIRED
                       PURPOSE "Required by the core KDE libraries and some critical kioslaves"
                      )


#optional features
find_package(X11)
if(X11_FOUND)
  macro_bool_to_01(X11_FOUND HAVE_X11)
  #X11_Xrender discovery is done by FindX11
  add_feature_info("X Rendering Extension (libXrender)" X11_Xrender_FOUND "Support for compositing, rendering operations, and alpha-blending. STRONGLY RECOMMENDED")
  macro_bool_to_01(X11_Xscreensaver_FOUND HAVE_XSCREENSAVER)
  macro_bool_to_01(X11_XSync_FOUND HAVE_XSYNC)
  add_feature_info("X Screensaver Extension (libXss)" HAVE_XSCREENSAVER "Support for KIdleTime (fallback mode)")
  add_feature_info("X Sync Extension (libXext)" HAVE_XSYNC "Efficient operation of KIdleTime. STRONGLY RECOMMENDED")

  if(NOT HAVE_XSYNC AND NOT HAVE_XSCREENSAVER)
    message(FATAL_ERROR "\nNeither the XSync (libXext) nor XScreensaver (libXss) development package was found.\nPlease install one of them (XSync is recommended)\n")
  endif(NOT HAVE_XSYNC AND NOT HAVE_XSCREENSAVER)

  #X11 Session Management (SM) is required
  #X11_SM_FOUND is set in FindX11, which is required by KDE4Internal
  if(UNIX AND NOT X11_SM_FOUND)
    message(FATAL_ERROR "\nThe X11 Session Management (SM) development package could not be found.\nPlease install libSM.\n")
  endif(UNIX AND NOT X11_SM_FOUND)
endif(X11_FOUND)

# find_package(OpenSSL)
# set_package_properties(OpenSSL PROPERTIES DESCRIPTION "Support for secure network communications (SSL and TLS)"
#                        URL "http://openssl.org"
#                        TYPE RECOMMENDED
#                        PURPOSE "KDE uses OpenSSL for the bulk of secure communications, including secure web browsing via HTTPS"
#                       )
# 
# find_package(Libintl)
# set_package_properties(Libintl PROPERTIES DESCRIPTION "Support for multiple languages"
#                        URL "http://www.gnu.org/software/gettext"
#                        TYPE RECOMMENDED
#                        PURPOSE "Enables KDE to be available in many different languages"
#                       )

find_package(QCA2 2.0.0)
set_package_properties(QCA2 PROPERTIES DESCRIPTION "Support for remote plasma widgets"
                       URL "http://delta.affinix.com/qca"
                       TYPE OPTIONAL
                      )

find_package(DBusMenuQt)
set_package_properties(DBusMenuQt PROPERTIES DESCRIPTION "Support for notification area menus via the DBusMenu protocol"
                       URL "https://launchpad.net/libdbusmenu-qt"
                       TYPE REQUIRED
                      )

################# Disallow in-source build #################

macro_ensure_out_of_source_build("kdelibs requires an out of source build. Please create a separate build directory and run 'cmake path_to_kdelibs [options]' there.")

# ... and warn in case of an earlier in-source build
set(generatedFileInSourceDir EXISTS ${kdelibs_SOURCE_DIR}/config.h OR EXISTS ${kdelibs_SOURCE_DIR}/config-prefix.h)
if(${generatedFileInSourceDir})
   message(STATUS "config.h or config-prefix.h exists in your source directory.")
   message(FATAL_ERROR "Please run git clean, it would seem that your source directory has generated files in it.")
endif(${generatedFileInSourceDir})
#########################################################################

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
remove_definitions(-DQT3_SUPPORT_WARNINGS -DQT3_SUPPORT)

add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
add_definitions(-DQT_NO_URL_CAST_FROM_STRING)

################# setup the include directories #################

# for including config.h and for includes like <kparts/foo.h>
include_directories( ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/interfaces)

if(QCA2_FOUND)
    include_directories(
        ${QCA2_INCLUDE_DIR}
    )
endif(QCA2_FOUND)



################# configure checks and create the configured files #################

if(WINCE)
  set(STATIC_LIBRARY ON)
  add_definitions(-DSTATIC_INSTALL_PATH=L\\\"/programme/kde\\\")
endif(WINCE)

if(STATIC_LIBRARY)
  set(LIBRARY_TYPE STATIC)
  add_definitions(-DKDELIBS_STATIC_LIBS)
  message(STATUS "Building kdelibs as static libraries")
else(STATIC_LIBRARY)
  set(LIBRARY_TYPE SHARED)
endif(STATIC_LIBRARY)

# now create config headers
configure_file(config-prefix.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-prefix.h )
configure_file(config-compiler.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-compiler.h )

# Needed for the calls to kconfig_add_kcfg_files in some modules
set(KCONFIG_KCFGC_EXECUTABLE kconfig_compiler)
include(KConfigMacros)


MACRO(installPackage dir component)
   SET(root ${ARGN})
   IF(NOT root)
      SET(root plasma/plasmoids)
   ENDIF(NOT root)
   install(DIRECTORY ${dir}/ DESTINATION ${DATA_INSTALL_DIR}/${root}/${component} PATTERN Messages.sh EXCLUDE)
   install(FILES ${dir}/metadata.desktop DESTINATION ${SERVICES_INSTALL_DIR} RENAME plasma-applet-${component}.desktop)
ENDMACRO(installPackage)

################# list the subdirectories #################

add_subdirectory( declarativeimports )
add_subdirectory( desktoptheme )
add_subdirectory( plasma )
add_subdirectory( scriptengines )
add_subdirectory( shell )

feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
