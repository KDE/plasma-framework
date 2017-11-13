add_subdirectory(accessdenied)
add_subdirectory(core)
add_subdirectory(plasmacomponents)
add_subdirectory(plasmaextracomponents)
add_subdirectory(platformcomponents)
add_subdirectory(calendar)

install(DIRECTORY plasmastyle/ DESTINATION ${KDE_INSTALL_QMLDIR}/QtQuick/Controls/Styles/Plasma)


#install the components as a QQC2 Style, as style for applications (mainly for Plasma Mobile)
install(DIRECTORY plasmacomponents3/ DESTINATION ${KDE_INSTALL_QMLDIR}/QtQuick/Controls.2/Plasma PATTERN qmldir EXCLUDE)

#install some of the componets as a separate import, to be used in plasmoids (some of them like ApplicationWindow are of no use for plasmoids)
install(FILES plasmacomponents3/BusyIndicator.qml
    plasmacomponents3/Button.qml
    plasmacomponents3/CheckBox.qml
    plasmacomponents3/CheckDelegate.qml
    plasmacomponents3/CheckIndicator.qml
    #combobox is not in a new window, but maybe better already than the broken qqc1 combobox?
    plasmacomponents3/ComboBox.qml
    plasmacomponents3/Container.qml
    plasmacomponents3/Control.qml
    plasmacomponents3/Dial.qml
    plasmacomponents3/Frame.qml
    plasmacomponents3/GroupBox.qml
    plasmacomponents3/ItemDelegate.qml
    plasmacomponents3/Label.qml

    plasmacomponents3/ProgressBar.qml
    plasmacomponents3/RadioButton.qml
    plasmacomponents3/RadioDelegate.qml
    plasmacomponents3/RadioIndicator.qml
    plasmacomponents3/RangeSlider.qml
    plasmacomponents3/ScrollBar.qml
    plasmacomponents3/Slider.qml
    plasmacomponents3/SpinBox.qml
    plasmacomponents3/Switch.qml
    plasmacomponents3/SwitchDelegate.qml
    plasmacomponents3/SwitchIndicator.qml
    plasmacomponents3/TabBar.qml
    plasmacomponents3/TabButton.qml
    plasmacomponents3/TextArea.qml
    plasmacomponents3/TextField.qml
    plasmacomponents3/ToolBar.qml
    plasmacomponents3/ToolButton.qml
    plasmacomponents3/qmldir

    DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/components.3)
install(DIRECTORY plasmacomponents3/private DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/components.3)
