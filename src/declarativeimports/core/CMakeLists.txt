if(HAVE_X11 AND XCB_XCB_FOUND AND XCB_COMPOSITE_FOUND AND XCB_DAMAGE_FOUND)
    set(HAVE_XCB_COMPOSITE TRUE)
else()
    set(HAVE_XCB_COMPOSITE FALSE)
endif()

configure_file(config-x11.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-x11.h)

ecm_add_qml_module(corebindingsplugin URI "org.kde.plasma.core")

target_sources(corebindingsplugin PRIVATE
    corebindingsplugin.cpp
    colorscope.cpp
    datamodel.cpp
    datasource.cpp
    #    runnermodel.cpp
    svgitem.cpp
    framesvgitem.cpp
    quicktheme.cpp
    tooltip.cpp
    tooltipdialog.cpp
    serviceoperationstatus.cpp
    iconitem.cpp
    units.cpp
    windowthumbnail.cpp
    managedtexturenode.cpp
    imagetexturescache.cpp
)

target_link_libraries(corebindingsplugin
        Qt6::Quick
        Qt6::Qml
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
else()

endif()

if(QtGui_Enabled_Features MATCHES "opengles2")
    target_link_libraries(corebindingsplugin Qt6::Gui_GLESv2)
endif()

install(FILES private/DefaultToolTip.qml DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/core/private)

ecm_finalize_qml_module(corebindingsplugin DESTINATION ${KDE_INSTALL_QMLDIR})
