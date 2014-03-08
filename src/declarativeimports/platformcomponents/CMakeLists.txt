set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_MODULE_PATH
    /opt/kf5/usr/kde/share/ECM-0.0.8/find-modules/
    ${CMAKE_MODULE_PATH}
    )

set(platformcomponents_SRCS
    platformextensionplugin.cpp
    application.cpp
    icondialog.cpp
    )

find_package(Qt5 5.2.0 REQUIRED NO_MODULE COMPONENTS Core DBus Quick Qml)

add_library(platformcomponentsplugin SHARED ${platformcomponents_SRCS})

qt5_use_modules(platformcomponentsplugin Core Widgets DBus Quick Qml)

target_link_libraries(
    platformcomponentsplugin
    Qt5::Core
    Qt5::DBus
    KF5::WindowSystem
    KF5::IconThemes
)

install(TARGETS platformcomponentsplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/platformcomponents)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/platformcomponents)

