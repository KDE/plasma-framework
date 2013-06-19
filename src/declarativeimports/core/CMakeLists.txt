project(corebindings)

INCLUDE_DIRECTORIES(
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src/plasma/includes
        ${CMAKE_BINARY_DIR}
        ${KDE4_INCLUDES}
        ${KDE4_INCLUDE_DIR}
        ${KDECLARATIVE_INCLUDE_DIR}
)

set(corebindings_SRCS
    corebindingsplugin.cpp
    theme.cpp
    datamodel.cpp
    datasource.cpp
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
        ${KDE4_KDEUI_LIBS}
        ${KDE4_KDECORE_LIBS}
        plasma)

install(TARGETS corebindingsplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/core)
install(FILES qmldir ToolTip.qml DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/core)

add_subdirectory(tests)
