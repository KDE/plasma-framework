set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(CheckCXXSourceCompiles)

add_definitions("-std=c++11")

set(CMAKE_MODULE_PATH
    /opt/kf5/usr/kde/share/ECM-0.0.8/find-modules/
    ${CMAKE_MODULE_PATH}
    )

set(solidx_LIB_SRCS
    main.cpp
    inputdevice.cpp
    inputdevicemodel.cpp

    backends/xlib/connection.cpp
    backends/xlib/inputdevicebackend.cpp
    )

include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckCXXSourceCompiles)

find_package(X11 REQUIRED)
find_package(XCB REQUIRED)
find_package(X11_XCB)
find_package(Qt5 5.2.0 REQUIRED NO_MODULE COMPONENTS Core Widgets)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
    ${XCB_INCLUDE_DIR}
    )

add_executable(SolidX ${solidx_LIB_SRCS})

qt5_use_modules(SolidX Core Widgets)

message("X11_X11_LIB                     ${X11_X11_LIB}")
message("X11_Xext_LIB                    ${X11_Xext_LIB}")
message("X11_ICE_LIB                     ${X11_ICE_LIB}")
message("X11_SM_LIB                      ${X11_SM_LIB}")
message("X11_Xcursor_LIB                 ${X11_Xcursor_LIB}")
message("X11_Xrandr_LIB                  ${X11_Xrandr_LIB}")
message("X11_Xdamage_LIB                 ${X11_Xdamage_LIB}")
message("X11_Xrender_LIB                 ${X11_Xrender_LIB}")
message("X11_Xfixes_LIB                  ${X11_Xfixes_LIB}")
message("X11_XCB_LIBRARIES               ${X11_XCB_LIBRARIES}")
message("XCB_XCB_LIBRARIES               ${XCB_XCB_LIBRARIES}")
message("XCB_XFIXES_LIBRARIES            ${XCB_XFIXES_LIBRARIES}")
message("XCB_DAMAGE_LIBRARIES            ${XCB_DAMAGE_LIBRARIES}")
message("XCB_COMPOSITE_LIBRARIES         ${XCB_COMPOSITE_LIBRARIES}")
message("XCB_SHAPE_LIBRARIES             ${XCB_SHAPE_LIBRARIES}")
message("XCB_SYNC_LIBRARIES              ${XCB_SYNC_LIBRARIES}")
message("XCB_RENDER_LIBRARIES            ${XCB_RENDER_LIBRARIES}")
message("XCB_RANDR_LIBRARIES             ${XCB_RANDR_LIBRARIES}")
message("XCB_KEYSYMS_LIBRARIES           ${XCB_KEYSYMS_LIBRARIES}")
message("XCB_ICCCM_LIBRARIES             ${XCB_ICCCM_LIBRARIES}")

target_link_libraries(
    SolidX
    ${X11_X11_LIB}
    ${X11_INPUT_LIB} -lXi
    ${XCB_XCB_LIBRARIES}
    ${X11_XCB_LIBRARIES}
)
# target_link_libraries(SolidX LINK_PUBLIC Qt5::Core Qt5::Gui ${XCB_XCB_LIBRARIES} ${X11_XCB_LIBRARIES})

