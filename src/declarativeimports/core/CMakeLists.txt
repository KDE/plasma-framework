project(corebindings)

add_definitions(-DHAVE_X11=${HAVE_X11})
if(XCB_XCB_FOUND AND XCB_COMPOSITE_FOUND AND XCB_DAMAGE_FOUND)
    add_definitions(-DHAVE_XCB_COMPOSITE=1)
    include_directories(
        ${XCB_XCB_INCLUDE_DIR}
        ${XCB_DAMAGE_INCLUDE_DIR}
        ${XCB_COMPOSITE_INCLUDE_DIR}
    )
endif()

set(corebindings_SRCS
    corebindingsplugin.cpp
    theme.cpp
    datamodel.cpp
    datasource.cpp
    dialogshadows.cpp
    #    runnermodel.cpp
    svgitem.cpp
    framesvgitem.cpp
    dialog.cpp
    tooltip.cpp
    tooltipdialog.cpp
    serviceoperationstatus.cpp
    dataenginebindings.cpp
    iconitem.cpp
    windowthumbnail.cpp
    )

add_library(corebindingsplugin SHARED ${corebindings_SRCS})
target_link_libraries(corebindingsplugin
        ${QT_QTSCRIPT_LIBRARY}
        ${Qt5Quick_LIBRARIES}
        ${Qt5Qml_LIBRARIES}
        ${KDeclarative_LIBRARIES}
        ${KCore_LIBRARIES}
        KF5::KIconThemes
        KF5::KWindowSystem
        plasma)

if(X11_FOUND)
  target_link_libraries(corebindingsplugin ${X11_LIBRARIES} ${XCB_XCB_LIBRARY} )
  target_link_libraries(corebindingsplugin Qt5::X11Extras)

  if(XCB_COMPOSITE_FOUND AND XCB_DAMAGE_FOUND)
    target_link_libraries(corebindingsplugin
        ${XCB_DAMAGE_LIBRARY}
        ${XCB_COMPOSITE_LIBRARY}
        )
  endif()

  if(OPENGL_FOUND)
    target_link_libraries(corebindingsplugin ${OPENGL_gl_LIBRARY})
  endif()
endif(X11_FOUND)

install(TARGETS corebindingsplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/core)
install(FILES qmldir ToolTip.qml DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/core)

add_subdirectory(tests)
