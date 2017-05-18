cmake_minimum_required(VERSION 2.8.12)

project(plasma-%{APPNAMELC})
 
find_package(ECM 1.4.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

find_package(KF5 REQUIRED COMPONENTS
    Plasma
    I18n
)

find_package(Qt5 CONFIG REQUIRED COMPONENTS
    Qml
    Gui
    Core
)

# plasmoid
plasma_install_package(package org.kde.%{APPNAMELC})

# qml extension plugin
add_subdirectory(plugin)
