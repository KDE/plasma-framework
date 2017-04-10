if(HAVE_X11 AND XCB_XCB_FOUND AND XCB_COMPOSITE_FOUND AND XCB_DAMAGE_FOUND)
    set(HAVE_XCB_COMPOSITE TRUE)
else()
    set(HAVE_XCB_COMPOSITE FALSE)
endif()

configure_file(config-x11.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-x11.h)

set(corebindings_SRCS
    corebindingsplugin.cpp
    colorscope.cpp
    datamodel.cpp
    datasource.cpp
    #    runnermodel.cpp
    svgitem.cpp
    fadingnode.cpp
    framesvgitem.cpp
    quicktheme.cpp
    tooltip.cpp
    tooltipdialog.cpp
    serviceoperationstatus.cpp
    iconitem.cpp
    units.cpp
    windowthumbnail.cpp
    )

qt5_add_resources(corebindings_SRCS shaders.qrc)

add_library(corebindingsplugin SHARED ${corebindings_SRCS})
target_link_libraries(corebindingsplugin
        Qt5::Quick
        Qt5::Qml
        KF5::Declarative
        KF5::IconThemes
        KF5::Service #for kplugininfo.h
        KF5::WindowSystem
        KF5::Plasma
        KF5::PlasmaQuick
        KF5::QuickAddons
)

if(HAVE_X11)
  target_link_libraries(corebindingsplugin ${X11_LIBRARIES} XCB::XCB )
  target_link_libraries(corebindingsplugin Qt5::X11Extras)

  if(XCB_COMPOSITE_FOUND AND XCB_DAMAGE_FOUND)
    target_link_libraries(corebindingsplugin
        XCB::DAMAGE
        XCB::COMPOSITE
        )
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

install(TARGETS corebindingsplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/core)
install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/core)

install(FILES private/DefaultToolTip.qml DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/core/private)

ecm_generate_qmltypes(org.kde.plasma.core 2.0 DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/core)
