ecm_add_qml_module(plasmaextracomponentsplugin VERSION 2.0 URI "org.kde.plasma.extras" GENERATE_PLUGIN_SOURCE)

target_sources(plasmaextracomponentsplugin PRIVATE
    qmenu.cpp
    qmenuitem.cpp
)

ecm_target_qml_sources(plasmaextracomponentsplugin SOURCES
    qml/ActionTextField.qml
    qml/BasicPlasmoidHeading.qml
    qml/DescriptiveLabel.qml
    qml/ExpandableListItem.qml
    qml/Heading.qml
    qml/Highlight.qml
    qml/ListItem.qml
    qml/ModelContextMenu.qml
    qml/PasswordField.qml
    qml/PlaceholderMessage.qml
    qml/PlasmoidHeading.qml
    qml/Representation.qml
    qml/SearchField.qml
)

ecm_target_qml_sources(plasmaextracomponentsplugin PATH animations SOURCES
    qml/animations/ActivateAnimation.qml
    qml/animations/AppearAnimation.qml
    qml/animations/DisappearAnimation.qml
    qml/animations/PressedAnimation.qml
    qml/animations/ReleasedAnimation.qml
)

ecm_target_qml_sources(plasmaextracomponentsplugin PRIVATE PATH private SOURCES
    qml/private/BackgroundMetrics.qml
)

target_link_libraries(plasmaextracomponentsplugin PRIVATE
        Qt6::Quick
        Qt6::Qml
        Qt6::Widgets
        KF6::ConfigWidgets
        KF6::Plasma)

ecm_finalize_qml_module(plasmaextracomponentsplugin DESTINATION ${KDE_INSTALL_QMLDIR})
