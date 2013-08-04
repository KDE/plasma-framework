
set(hardware_common_SRCS
    inputdevice.cpp
    inputdevicemodel.cpp
    interface.cpp
    hardwareextensionplugin.cpp

    backends/abstractinputdevicebackend.cpp

    backends/xlib/xlibconnection.cpp
    backends/xlib/xlibinputdevicebackend.cpp
    backends/fake/fakeinputdevicebackend.cpp
    )

set(hardware_LIB_SRCS
    ${hardware_common_SRCS}
    hardwareextensionplugin.cpp
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

add_library(hardwareextensionplugin SHARED ${hardware_LIB_SRCS})

qt5_use_modules(hardwareextensionplugin Core Widgets Declarative DBus Quick Qml)

target_link_libraries(
    hardwareextensionplugin
    ${X11_X11_LIB}
    ${X11_INPUT_LIB} -lXi
    ${XCB_XCB_LIBRARIES}
    ${X11_XCB_LIBRARIES}
    Qt5::Core
    Qt5::Widgets
    Qt5::DBus
    Qt5::Declarative
)

install(TARGETS hardwareextensionplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/hardware)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/hardware)

