if(HAVE_X11 AND XCB_XCB_FOUND AND XCB_COMPOSITE_FOUND AND XCB_DAMAGE_FOUND)
    set(HAVE_XCB_COMPOSITE TRUE)
else()
    set(HAVE_XCB_COMPOSITE FALSE)
endif()

configure_file(config-x11.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-x11.h)

ecm_add_qml_module(corebindingsplugin URI "org.kde.plasma.core")

qt_add_resources(RESOURCES private/resources.qrc)

target_sources(corebindingsplugin PRIVATE
    action.cpp
    corebindingsplugin.cpp
    datamodel.cpp
    quicktheme.cpp
    tooltip.cpp
    tooltipdialog.cpp
    windowthumbnail.cpp
    ${RESOURCES}
)

target_link_libraries(corebindingsplugin
        Qt6::Quick
        Qt6::Qml
        Qt6::Widgets
        KF6::IconThemes
        KF6::I18n
        Qt6::Svg
        KF6::WindowSystem
        KF6::Plasma
        KF6::PlasmaQuick
)

if(HAVE_X11)
  target_link_libraries(corebindingsplugin ${X11_LIBRARIES} XCB::XCB Qt6::GuiPrivate)

  if(XCB_COMPOSITE_FOUND AND XCB_DAMAGE_FOUND)
    target_link_libraries(corebindingsplugin
        XCB::DAMAGE
        XCB::COMPOSITE
        )
  endif()

  if(HAVE_GLX)
    target_link_libraries(corebindingsplugin OpenGL::GLX)
  endif()
endif()

if(HAVE_EGL)
    target_link_libraries(corebindingsplugin OpenGL::EGL)
endif()

ecm_finalize_qml_module(corebindingsplugin DESTINATION ${KDE_INSTALL_QMLDIR})
