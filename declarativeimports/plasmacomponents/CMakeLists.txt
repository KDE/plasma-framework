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
#it does install some files on top of the old ones, has to be done file by file since if some component from the generic set is more recent than the specifc ones, it wouldn't be overwritten

install(TARGETS plasmacomponentsplugin DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/components)


install(FILES qml/AppManager.js DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/components)
install(FILES qml/BusyIndicator.qml DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/components)
install(FILES qml/ButtonColumn.qml DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/components)
install(FILES qml/ButtonGroup.js DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/components)
install(FILES qml/Button.qml DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/components)
install(FILES qml/ButtonRow.qml DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/components)
install(FILES qml/CheckBox.qml DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/components)
install(FILES qml/CommonDialog.qml DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/components)
install(FILES qml/Dialog.qml DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/components)
install(FILES qml/DualStateButton.qml DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/components)
install(FILES qml/Highlight.qml DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/components)
install(FILES qml/IconLoader.qml DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/components)
install(FILES qml/Label.qml DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/components)
install(FILES qml/ListItem.qml DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/components)
install(FILES qml/Page.qml DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/components)
install(FILES qml/PageStack.js DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/components)
install(FILES qml/PageStack.qml DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/components)
install(FILES qml/ProgressBar.qml DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/components)
install(FILES qml/RadioButton.qml DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/components)
install(FILES qml/ScrollBarDelegate.qml DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/components)
install(FILES qml/ScrollDecoratorDelegate.qml DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/components)
install(FILES qml/SectionScroller.qml DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/components)
install(FILES qml/SelectionDialog.qml DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/components)
install(FILES qml/Slider.qml DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/components)
install(FILES qml/Switch.qml DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/components)
install(FILES qml/TabBarLayout.qml DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/components)
install(FILES qml/TabBar.qml DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/components)
install(FILES qml/TabButton.qml DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/components)
install(FILES qml/TabGroup.js DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/components)
install(FILES qml/TextArea.qml DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/components)
install(FILES qml/TextField.qml DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/components)
install(FILES qml/ToolBarLayout.qml DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/components)
install(FILES qml/ToolBar.qml DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/components)
install(FILES qml/ToolButton.qml DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/components)


#install platform overrides
install(DIRECTORY platformcomponents/touch/ DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/components)
