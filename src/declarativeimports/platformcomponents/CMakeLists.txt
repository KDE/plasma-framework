set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(platformcomponents_SRCS
    platformextensionplugin.cpp
    application.cpp
    icondialog.cpp
    )

find_package(Qt5 5.2.0 REQUIRED NO_MODULE COMPONENTS Core DBus Qml)

add_library(platformcomponentsplugin SHARED ${platformcomponents_SRCS})

target_link_libraries(
    platformcomponentsplugin
    Qt5::Core
    Qt5::DBus
    Qt5::Qml
    KF5::WindowSystem
    KF5::IconThemes
)

install(TARGETS platformcomponentsplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/platformcomponents)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/platformcomponents)

