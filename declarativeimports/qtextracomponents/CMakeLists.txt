project(qtextracomponents)

include(KDE4Defaults)

set(qtextracomponents_SRCS
    qtextracomponentsplugin.cpp
    qpixmapitem.cpp
    qimageitem.cpp
    qiconitem.cpp
    qmenu.cpp
    )

INCLUDE_DIRECTORIES(
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}
        ${KDE4_INCLUDES}
)

qt4_automoc(${qtextracomponents_SRCS})


add_library(qtextracomponentsplugin SHARED ${qtextracomponents_SRCS})
target_link_libraries(qtextracomponentsplugin ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTDECLARATIVE_LIBRARY})

install(TARGETS qtextracomponentsplugin DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/qtextracomponents)

install(FILES qmldir DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/qtextracomponents)
