cmake_minimum_required(VERSION 3.16)

set(KF_VERSION "5.103.0") # handled by release scripts
set(KF_DEP_VERSION "5.103.0") # handled by release scripts
project(Plasma VERSION ${KF_VERSION})

# ECM setup
include(FeatureSummary)
find_package(ECM 5.103.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEGitCommitHooks)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)

include(ECMGenerateExportHeader)
include(ECMGenerateHeaders)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)
include(ECMQtDeclareLoggingCategory)
include(ECMAddQch)
include(KDEPackageAppTemplates)
include(ECMGenerateQmlTypes)
include(ECMSetupQtPluginMacroNames)
include(ECMMarkNonGuiExecutable)
include(ECMDeprecationSettings)

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

ecm_setup_version(PROJECT
                  VARIABLE_PREFIX PLASMA
                  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/plasma_version.h"
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5PlasmaConfigVersion.cmake"
                  SOVERSION 5)

################# now find all used packages #################

set (REQUIRED_QT_VERSION 5.15.2)

find_package(Qt${QT_MAJOR_VERSION} ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE COMPONENTS Quick Gui Sql Qml Svg QuickControls2)
if (QT_MAJOR_VERSION EQUAL "6")
    find_package(Qt6 ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE COMPONENTS ShaderTools)
endif()

find_package(KF5 ${KF_DEP_VERSION} REQUIRED
    COMPONENTS
        Activities
        Archive
        Config
        ConfigWidgets
        CoreAddons
        DBusAddons
        Declarative
        GlobalAccel
        GuiAddons
        I18n
        IconThemes
        KIO
        WindowSystem
        XmlGui
        Notifications
        Package
        Kirigami2
    OPTIONAL_COMPONENTS
        Wayland
        DocTools
)
if(NOT EXCLUDE_DEPRECATED_BEFORE_AND_AT STREQUAL "CURRENT" AND
   EXCLUDE_DEPRECATED_BEFORE_AND_AT VERSION_LESS 5.94.0)
    find_package(KF5 ${KF_DEP_VERSION} REQUIRED COMPONENTS Service)
endif()

set_package_properties(KF5Wayland PROPERTIES DESCRIPTION "Integration with the Wayland compositor"
                       TYPE OPTIONAL
                      )
if(KF5Wayland_FOUND)
  set(HAVE_KWAYLAND 1)
endif()

set_package_properties(KF5DocTools PROPERTIES DESCRIPTION "Tools to generate documentation"
                       TYPE OPTIONAL
                      )

option(WITHOUT_X11 "Build without X11 support (skips finding X11)." OFF)
if(NOT WITHOUT_X11)
    #optional features
    find_package(X11 MODULE)
    set_package_properties(X11 PROPERTIES DESCRIPTION "X11 libraries"
                        URL "https://www.x.org/"
                        TYPE OPTIONAL
                        )
    find_package(XCB MODULE COMPONENTS XCB COMPOSITE DAMAGE SHAPE XFIXES RENDER)
    set_package_properties(XCB PROPERTIES DESCRIPTION "X protocol C-language Binding"
                        URL "https://xcb.freedesktop.org/"
                        TYPE OPTIONAL
                        )
    if(X11_FOUND AND XCB_XCB_FOUND)
        set(HAVE_X11 1)
        if (QT_MAJOR_VERSION STREQUAL "5")
            find_package(Qt5X11Extras ${REQUIRED_QT_VERSION} NO_MODULE)
        elseif (QT_MAJOR_VERSION STREQUAL "6")
            find_package(Qt${QT_MAJOR_VERSION} ${REQUIRED_QT_VERSION} CONFIG REQUIRED Gui) # qtx11extras_p.h
        endif()
        #X11_Xrender discovery is done by FindX11
        #add_feature_info("X Rendering Extension (libXrender)" X11_Xrender_FOUND "Support for compositing, rendering operations, and alpha-blending. STRONGLY RECOMMENDED")
        set(HAVE_XCB_SHAPE ${XCB_SHAPE_FOUND})
    else()
        set(HAVE_X11 0)
        set(HAVE_XCB_SHAPE 0)
    endif()
endif()

find_package(OpenGL)
set_package_properties(OpenGL PROPERTIES DESCRIPTION "The OpenGL libraries"
                       URL "https://www.opengl.org/"
                       TYPE OPTIONAL
                      )

find_package(EGL)
set_package_properties(EGL PROPERTIES
                       PURPOSE "Fallback when OpenGL not available because of missing GLVND"
                       TYPE OPTIONAL
                      )

get_target_property(QtGui_Enabled_Features Qt${QT_MAJOR_VERSION}::Gui QT_ENABLED_PUBLIC_FEATURES)
if(OpenGL_GLX_FOUND AND HAVE_X11 AND (QtGui_Enabled_Features MATCHES "opengl"))
    add_feature_info(GLX OpenGL_GLX_FOUND "OpenGL GLX libraries.")
    set(HAVE_GLX 1)
else()
    set(HAVE_GLX 0)
endif()


# OpenGL_EGL_FOUND is defined by FindOpenGL
if(TARGET OpenGL::EGL)
    add_feature_info(EGL OpenGL_EGL_FOUND
                     "A platform-agnostic mechanism for creating rendering surfaces for use with other graphics libraries, such as OpenGL|ES and OpenVG.")
    set(HAVE_EGL 1)
else()
    set(HAVE_EGL 0)
    message(STATUS "OpenGL::EGL is not available")
endif()


#########################################################################

# TODO KF6 remove
ecm_setup_qtplugin_macro_names(
    JSON_NONE
        "K_EXPORT_PLASMA_SERVICE"
        "K_EXPORT_PLASMA_APPLET"
        "K_EXPORT_PLASMA_PACKAGE"
        "K_EXPORT_PLASMA_APPLETSCRIPTENGINE"
        "K_EXPORT_PLASMA_DATAENGINESCRIPTENGINE"
        "K_EXPORT_PLASMA_DATAENGINE"
    JSON_ARG2
        "K_EXPORT_PLASMA_PACKAGE_WITH_JSON"
    JSON_ARG3
        "K_EXPORT_PLASMA_SERVICE_WITH_JSON"
        "K_EXPORT_PLASMA_APPLET_WITH_JSON"
        "K_EXPORT_PLASMA_APPLETSCRIPTENGINE_WITH_JSON"
        "K_EXPORT_PLASMA_DATAENGINESCRIPTENGINE_WITH_JSON"
        "K_EXPORT_PLASMA_CONTAINMENTACTIONS_WITH_JSON"
        "K_EXPORT_PLASMA_DATAENGINE_WITH_JSON"
    CONFIG_CODE_VARIABLE
        PACKAGE_SETUP_AUTOMOC_VARIABLES
)

ecm_set_disabled_deprecation_versions(
    QT 5.14 # needed for qmlRegisterInterface
    KF 5.95
    KCOREADDONS 5.88 # KPluginMetaData::serviceTypes
    KSERVICE 5.86 # KService::pluginKeyword
    KDECLARATIVE 5.88 # KDeclarative::ConfigPropertyMap
)

#add_definitions(-Wno-deprecated)

include(KF5PlasmaMacros.cmake)

#########################################################################


option(BUILD_EXAMPLES "Build and install Plasma examples." OFF)
option(BUILD_COVERAGE "Build Plasma Frameworks with gcov support" OFF)

if(BUILD_COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lgcov")
endif()

# make plasma_version.h available
include_directories(${CMAKE_CURRENT_BINARY_DIR})

################# list the subdirectories #################
if (KF5DocTools_FOUND)
    add_subdirectory(docs)
endif()
add_definitions(-DTRANSLATION_DOMAIN=\"libplasma5\")
ki18n_install(po)
if (KF5DocTools_FOUND)
    kdoctools_install(po)
endif()
add_subdirectory(src)

if (BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if (BUILD_TESTING)
    add_subdirectory(autotests)
    add_subdirectory(tests)
endif()
add_subdirectory(templates)

################ create PlasmaConfig.cmake and install it ###########################

# create a Config.cmake and a ConfigVersion.cmake file and install them

set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5Plasma")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF5Plasma_QCH
        FILE KF5PlasmaQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF5PlasmaQchTargets.cmake\")")
endif()

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5PlasmaConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5PlasmaConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
  PATH_VARS  CMAKE_INSTALL_PREFIX
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
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/Plasma COMPONENT Devel )

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
