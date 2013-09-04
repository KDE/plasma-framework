project(corebindings)

INCLUDE_DIRECTORIES(
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src/plasma/includes
        ${CMAKE_BINARY_DIR}
        ${KDE4_INCLUDES}
        ${KDE4_INCLUDE_DIR}
        ${KDECLARATIVE_INCLUDE_DIR}
)

add_definitions(-DHAVE_X11=${HAVE_X11})

set(corebindings_SRCS
    corebindingsplugin.cpp
    theme.cpp
    datamodel.cpp
    datasource.cpp
    dialogshadows.cpp
    runnermodel.cpp
    svgitem.cpp
    framesvgitem.cpp
    dialog.cpp
    tooltip.cpp
    tooltipdialog.cpp
    serviceoperationstatus.cpp
    dataenginebindings.cpp
    iconitem.cpp
    )

add_library(corebindingsplugin SHARED ${corebindings_SRCS})
target_link_libraries(corebindingsplugin
        ${QT_QTSCRIPT_LIBRARY}
        ${Qt5Quick_LIBRARIES}
        ${Qt5Qml_LIBRARIES}
        ${KDECLARATIVE_LIBRARIES}
        ${KCore_LIBRARIES}
        ${KDE4Support_LIBRARIES}
        KF5::KIconThemes
        KF5::KWindowSystem
        plasma)

if(X11_FOUND)
  target_link_libraries(corebindingsplugin ${X11_LIBRARIES} ${XCB_XCB_LIBRARY} )
  target_link_libraries(corebindingsplugin Qt5::X11Extras)
endif(X11_FOUND)

install(TARGETS corebindingsplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/core)
install(FILES qmldir ToolTip.qml DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/core)

add_subdirectory(tests)
