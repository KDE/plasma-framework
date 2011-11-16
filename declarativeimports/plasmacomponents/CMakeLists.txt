project(plasmacomponents)

set(plasmacomponents_SRCS
    plasmacomponentsplugin.cpp
    qrangemodel.cpp
    enums.cpp
    qmenu.cpp
    qmenuitem.cpp
    kdialogproxy.cpp
    )

INCLUDE_DIRECTORIES(
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}
        ${KDE4_INCLUDES}
)

qt4_automoc(${plasmacomponents_SRCS})


add_library(plasmacomponentsplugin SHARED ${plasmacomponents_SRCS})
target_link_libraries(plasmacomponentsplugin ${QT_QTCORE_LIBRARY} ${QT_QTDECLARATIVE_LIBRARY} ${QT_QTGUI_LIBRARY} ${KDE4_KDEUI_LIBRARY})

install(TARGETS plasmacomponentsplugin DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/plasma/components)

install(DIRECTORY qml/ DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/plasma/components)



#The platform specific stuff, overwrites a copy of the desktop one
#it does install some files on top of the old ones, it's pretty hackyu but it's intended since there are no more elegant ways to produce a fallback during a qml import from the most specific files to the general ones if specific were not found

install(TARGETS plasmacomponentsplugin DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/components)
install(DIRECTORY qml/ DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/components)
install(DIRECTORY platformcomponents/touch/ DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/components)
