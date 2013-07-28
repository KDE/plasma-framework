project(desktopshell)

set(desktopshell_SRCS
    desktopshellhandler.cpp
    desktopshellplugin.cpp
    )


add_library(desktopshellplugin SHARED ${desktopshell_SRCS})

target_link_libraries(desktopshellplugin
        ${QT_QTCORE_LIBRARY}
        ${Qt5Qml_LIBRARIES}
        ${KDE4_KIO_LIBS}
        ${KDE4Support_LIBRARIES}
        )

install(TARGETS desktopshellplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/shells/desktop)
install(FILES qmldir Handler.qml DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/shells/desktop)

