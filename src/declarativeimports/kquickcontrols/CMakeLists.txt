project(kquickcontrols)

install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/kquickcontrols)
install(FILES KeySequenceItem.qml DESTINATION ${QML_INSTALL_DIR}/org/kde/kquickcontrols)

add_subdirectory(private)

