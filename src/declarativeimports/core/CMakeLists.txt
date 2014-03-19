project(corebindings)
if(HAVE_X11 AND XCB_XCB_FOUND AND XCB_COMPOSITE_FOUND AND XCB_DAMAGE_FOUND)
    add_definitions(-DHAVE_XCB_COMPOSITE=1)
    include_directories(
        ${XCB_XCB_INCLUDE_DIR}
        ${XCB_DAMAGE_INCLUDE_DIR}
        ${XCB_COMPOSITE_INCLUDE_DIR}
    )
else()
    add_definitions(-DHAVE_XCB_COMPOSITE=0)
endif()
if(HAVE_X11 AND XCB_XCB_FOUND AND XCB_SHAPE_FOUND)
    add_definitions(-DHAVE_XCB_SHAPE=1)
    include_directories(${XCB_SHAPE_INCLUDE_DIR})
else()
    add_definitions(-DHAVE_XCB_SHAPE=0)
endif()

set(corebindings_SRCS
    corebindingsplugin.cpp
    datamodel.cpp
    datasource.cpp
    #    runnermodel.cpp
    svgitem.cpp
    framesvgitem.cpp
    tooltip.cpp
    tooltipdialog.cpp
    formats.cpp
    serviceoperationstatus.cpp
    dataenginebindings.cpp
    iconitem.cpp
    units.cpp
    windowthumbnail.cpp
    )

add_library(corebindingsplugin SHARED ${corebindings_SRCS})
target_link_libraries(corebindingsplugin
        Qt5::Script
        Qt5::Quick
        Qt5::Qml
        KF5::Declarative
        KF5::CoreAddons
        KF5::IconThemes
        KF5::Service #for kplugininfo.h
        KF5::WindowSystem
        KF5::Plasma
        KF5::PlasmaQuick)

if(HAVE_X11)
  target_link_libraries(corebindingsplugin ${X11_LIBRARIES} ${XCB_XCB_LIBRARY} )
  target_link_libraries(corebindingsplugin Qt5::X11Extras)

  if(XCB_COMPOSITE_FOUND AND XCB_DAMAGE_FOUND)
    target_link_libraries(corebindingsplugin
        ${XCB_DAMAGE_LIBRARY}
        ${XCB_COMPOSITE_LIBRARY}
        )
  endif()
  if(XCB_SHAPE_FOUND)
    target_link_libraries(corebindingsplugin ${XCB_SHAPE_LIBRARY})
  endif()

  if(HAVE_GLX)
    target_link_libraries(corebindingsplugin ${OPENGL_gl_LIBRARY})
  endif()
endif()

if(HAVE_EGL)
    target_link_libraries(corebindingsplugin EGL::EGL)
endif()

if (${Qt5Gui_OPENGL_IMPLEMENTATION} STREQUAL "GLESv2")
    target_link_libraries(corebindingsplugin Qt5::Gui_GLESv2)
endif()

install(TARGETS corebindingsplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/core)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/core)

install(FILES private/DefaultToolTip.qml DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/core/private)

add_subdirectory(tests)
