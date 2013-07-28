project(activeshell)

set(activeshell_SRCS
    activeshellhandler.cpp
    activeshellplugin.cpp
    )


add_library(activeshellplugin SHARED ${activeshell_SRCS})

target_link_libraries(activeshellplugin
        ${QT_QTCORE_LIBRARY}
        ${Qt5Qml_LIBRARIES}
        ${KDE4_KIO_LIBS}
        ${KDE4Support_LIBRARIES}
        )

install(TARGETS activeshellplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/shells/active)
install(FILES qmldir Handler.qml DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/shells/active)

