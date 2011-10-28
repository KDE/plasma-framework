project(plasmacomponents)

set(plasmacomponents_SRCS
    plasmacomponentsplugin.cpp
    qrangemodel.cpp
    enums.cpp
    qmenu.cpp
    qmenuitem.cpp
    )

INCLUDE_DIRECTORIES(
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}
        ${KDE4_INCLUDES}
)

qt4_automoc(${plasmacomponents_SRCS})


add_library(plasmacomponentsplugin SHARED ${plasmacomponents_SRCS})
target_link_libraries(plasmacomponentsplugin ${QT_QTCORE_LIBRARY} ${QT_QTDECLARATIVE_LIBRARY} ${QT_QTGUI_LIBRARY})

install(TARGETS plasmacomponentsplugin DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/plasma/components)

install(DIRECTORY qml/ DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/plasma/components)
