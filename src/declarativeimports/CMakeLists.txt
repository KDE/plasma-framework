add_subdirectory(accessdenied)
add_subdirectory(core)
add_subdirectory(plasmacomponents)
add_subdirectory(plasmaextracomponents)
add_subdirectory(platformcomponents)
add_subdirectory(calendar)

install(DIRECTORY plasmastyle/ DESTINATION ${KDE_INSTALL_QMLDIR}/QtQuick/Controls/Styles/Plasma)

install(DIRECTORY kirigamiplasmastyle/ DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/kirigami.2/styles/Plasma)

install(DIRECTORY kirigamiplasmadesktopstyle/ DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/kirigami.2/styles/org.kde.desktop.plasma)


math(EXPR QQC2_VERSION "${Qt5QuickControls2_VERSION_MINOR} - 7")
set(QQC2_VERSION "2.${QQC2_VERSION}")


# Find all the source qml files
FILE(GLOB_RECURSE inFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/plasmacomponents3/*")

    #for each file, replace @QQC2_VERSION@ with the version we found
FOREACH(infileName ${inFiles})
    configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/${infileName}"
        "${CMAKE_CURRENT_BINARY_DIR}/${infileName}"
        INSTALL_DESTINATION  "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5Plasma"
        PATH_VARS  QQC2_VERSION
    )
ENDFOREACH(infileName)

#install the components as a QQC2 Style, as style for applications (mainly for Plasma Mobile)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/plasmacomponents3/ DESTINATION ${KDE_INSTALL_QMLDIR}/QtQuick/Controls.2/Plasma)

#install some of the componets as a separate import, to be used in plasmoids (some of them like ApplicationWindow are of no use for plasmoids)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/plasmacomponents3/BusyIndicator.qml
    ${CMAKE_CURRENT_BINARY_DIR}/plasmacomponents3/Button.qml
    ${CMAKE_CURRENT_BINARY_DIR}/plasmacomponents3/CheckBox.qml
    ${CMAKE_CURRENT_BINARY_DIR}/plasmacomponents3/CheckDelegate.qml
    ${CMAKE_CURRENT_BINARY_DIR}/plasmacomponents3/CheckIndicator.qml
    #combobox is not in a new window, but maybe better already than the broken qqc1 combobox?
    ${CMAKE_CURRENT_BINARY_DIR}/plasmacomponents3/ComboBox.qml
    ${CMAKE_CURRENT_BINARY_DIR}/plasmacomponents3/Container.qml
    ${CMAKE_CURRENT_BINARY_DIR}/plasmacomponents3/Control.qml
    ${CMAKE_CURRENT_BINARY_DIR}/plasmacomponents3/Dial.qml
    ${CMAKE_CURRENT_BINARY_DIR}/plasmacomponents3/Frame.qml
    ${CMAKE_CURRENT_BINARY_DIR}/plasmacomponents3/GroupBox.qml
    ${CMAKE_CURRENT_BINARY_DIR}/plasmacomponents3/ItemDelegate.qml
    ${CMAKE_CURRENT_BINARY_DIR}/plasmacomponents3/Label.qml

    ${CMAKE_CURRENT_BINARY_DIR}/plasmacomponents3/ProgressBar.qml
    ${CMAKE_CURRENT_BINARY_DIR}/plasmacomponents3/RadioButton.qml
    ${CMAKE_CURRENT_BINARY_DIR}/plasmacomponents3/RadioDelegate.qml
    ${CMAKE_CURRENT_BINARY_DIR}/plasmacomponents3/RadioIndicator.qml
    ${CMAKE_CURRENT_BINARY_DIR}/plasmacomponents3/RangeSlider.qml
    ${CMAKE_CURRENT_BINARY_DIR}/plasmacomponents3/ScrollBar.qml
    ${CMAKE_CURRENT_BINARY_DIR}/plasmacomponents3/Slider.qml
    ${CMAKE_CURRENT_BINARY_DIR}/plasmacomponents3/SpinBox.qml
    ${CMAKE_CURRENT_BINARY_DIR}/plasmacomponents3/Switch.qml
    ${CMAKE_CURRENT_BINARY_DIR}/plasmacomponents3/SwitchDelegate.qml
    ${CMAKE_CURRENT_BINARY_DIR}/plasmacomponents3/SwitchIndicator.qml
    ${CMAKE_CURRENT_BINARY_DIR}/plasmacomponents3/TabBar.qml
    ${CMAKE_CURRENT_BINARY_DIR}/plasmacomponents3/TabButton.qml
    ${CMAKE_CURRENT_BINARY_DIR}/plasmacomponents3/TextArea.qml
    ${CMAKE_CURRENT_BINARY_DIR}/plasmacomponents3/TextField.qml
    ${CMAKE_CURRENT_BINARY_DIR}/plasmacomponents3/ToolBar.qml
    ${CMAKE_CURRENT_BINARY_DIR}/plasmacomponents3/qmldir

    DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/components.3)

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/plasmacomponents3/private DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/components.3)

