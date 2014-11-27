#find_package(KF5Declarative REQUIRED NO_MODULE)

set(plasmacomponents_SRCS
    #fullscreensheet.cpp
    #fullscreenwindow.cpp
    plasmacomponentsplugin.cpp
    qrangemodel.cpp
    enums.cpp
    qmenu.cpp
    qmenuitem.cpp
    )

add_library(plasmacomponentsplugin SHARED ${plasmacomponents_SRCS})
target_link_libraries(plasmacomponentsplugin
        Qt5::Core
        Qt5::Quick
        Qt5::Qml
        Qt5::Gui
        KF5::Declarative
        KF5::Plasma
        KF5::IconThemes
        KF5::Service #for kplugininfo.h
)


install(TARGETS plasmacomponentsplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/components)

install(DIRECTORY qml/ DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/components)

install(DIRECTORY qml/styles/ DESTINATION ${QML_INSTALL_DIR}/QtQuick/Controls/Styles/Plasma)
#FIXME: hack
install(DIRECTORY qml/private DESTINATION ${QML_INSTALL_DIR}/QtQuick/Controls/Styles/)
