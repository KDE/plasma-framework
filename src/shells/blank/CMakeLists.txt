project(blankshell)

set(blankshell_SRCS
    blankshellhandler.cpp
    blankshellplugin.cpp
    )


add_library(blankshellplugin SHARED ${blankshell_SRCS})

target_link_libraries(blankshellplugin
        ${QT_QTCORE_LIBRARY}
        ${Qt5Qml_LIBRARIES}
        ${KDE4_KIO_LIBS}
        ${KDE4Support_LIBRARIES}
        )

install(TARGETS blankshellplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/shells/blank)
install(FILES qmldir Handler.qml DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/shells/blank)

