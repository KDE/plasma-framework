project(corebindings)

INCLUDE_DIRECTORIES(
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}
        ${KDE4_INCLUDES}
        ${KDE4_INCLUDE_DIR}
        ${KDECLARATIVE_INCLUDE_DIR}
)

set(corebindings_SRCS
#     declarativeitemcontainer.cpp
    corebindingsplugin.cpp
#     dataengineconsumer.cpp
     theme.cpp
#     datamodel.cpp
#     datasource.cpp
#     runnermodel.cpp
    svgitem.cpp
    framesvgitem.cpp
#     dialog.cpp
#     tooltip.cpp
#     dataenginebindings.cpp
    iconitem.cpp
    )


qt4_automoc(${corebindings_SRCS})

# Qt installs plugins here:
# /home/sebas/kf5/qt5/qtdeclarative/src/imports/window/plugins.qmltypes /home/sebas/kf5/install/qml/QtQuick/Window.2/
set(IMPORTS_INSTALL_DIR "qml")
message("!!! Forcing Imports install location: ${IMPORTS_INSTALL_DIR}")


add_library(corebindingsplugin SHARED ${corebindings_SRCS})
target_link_libraries(corebindingsplugin
        ${QT_QTSCRIPT_LIBRARY}
        ${Qt5Quick_LIBRARIES}
        ${Qt5Qml_LIBRARIES}
        ${KDECLARATIVE_LIBRARIES}
        ${KDE4_KDEUI_LIBS}
        kdecore
        plasma)

install(TARGETS corebindingsplugin DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/plasma/core)
install(FILES qmldir DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/plasma/core)

#add_subdirectory(tests)
