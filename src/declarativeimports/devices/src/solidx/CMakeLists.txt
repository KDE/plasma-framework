set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(CheckCXXSourceCompiles)

add_definitions("-std=c++11")

set(CMAKE_MODULE_PATH
    /opt/kf5/usr/kde/share/ECM-0.0.8/find-modules/
    ${CMAKE_MODULE_PATH}
    )

set(solidx_common_SRCS
    inputdevice.cpp
    inputdevicemodel.cpp
    interface.cpp
    solidxextensionplugin.cpp

    backends/abstractinputdevicebackend.cpp

    backends/xlib/xlibconnection.cpp
    backends/xlib/xlibinputdevicebackend.cpp
    backends/fake/fakeinputdevicebackend.cpp
    )

set(solidx_LIB_SRCS
    ${solidx_common_SRCS}
    solidxextensionplugin.cpp
    )

include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckCXXSourceCompiles)

find_package(X11 REQUIRED)
find_package(XCB REQUIRED)
find_package(X11_XCB)
find_package(Qt5 5.2.0 REQUIRED NO_MODULE COMPONENTS Core Widgets DBus Declarative Quick Qml)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
    ${XCB_INCLUDE_DIR}
    )

add_library(solidxextensionplugin SHARED ${solidx_LIB_SRCS})

qt5_use_modules(solidxextensionplugin Core Widgets Declarative DBus Quick Qml)

target_link_libraries(
    solidxextensionplugin
    ${X11_X11_LIB}
    ${X11_INPUT_LIB} -lXi
    ${XCB_XCB_LIBRARIES}
    ${X11_XCB_LIBRARIES}
    Qt5::Core
    Qt5::Widgets
    Qt5::DBus
    Qt5::Declarative
)

install(TARGETS solidxextensionplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/solidx)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/solidx)

