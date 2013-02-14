project(qtextracomponents)

add_subdirectory(tests)

set(qtextracomponents_SRCS
    qtextracomponentsplugin.cpp
    qpixmapitem.cpp
    qimageitem.cpp
    qiconitem.cpp
    mouseeventlistener.cpp
    columnproxymodel.cpp
    )

INCLUDE_DIRECTORIES(
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}
        ${KDE4_INCLUDES}
)

qt4_automoc(${qtextracomponents_SRCS})

add_library(qtextracomponentsplugin SHARED ${qtextracomponents_SRCS})

target_link_libraries(qtextracomponentsplugin
        ${QT_QTCORE_LIBRARY}
        ${Qt5Quick_LIBRARIES}
        ${Qt5Qml_LIBRARIES}
        ${QT_QTGUI_LIBRARY}
        ${KDE4_KDEUI_LIBRARY}
        ${KDECLARATIVE_LIBRARIES}
        plasma)

install(TARGETS qtextracomponentsplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/qtextracomponents)

install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/qtextracomponents)
