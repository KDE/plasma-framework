project(plasmacomponents)

#find_package(KF5Declarative REQUIRED NO_MODULE)

set(plasmacomponents_SRCS
    #fullscreensheet.cpp
    #fullscreenwindow.cpp
    plasmacomponentsplugin.cpp
    qrangemodel.cpp
    enums.cpp
    qmenu.cpp
    qmenuitem.cpp
    units.cpp
    #../core/declarativeitemcontainer.cpp
    )

add_library(plasmacomponentsplugin SHARED ${plasmacomponents_SRCS})
target_link_libraries(plasmacomponentsplugin
        Qt5::Core
        Qt5::Quick
        Qt5::Qml
        Qt5::Gui
        KF5::Declarative
        KF5::Plasma)


install(TARGETS plasmacomponentsplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/components)

install(DIRECTORY qml/ DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/components)



#The platform specific stuff, overwrites a copy of the desktop one
#it does install some files on top of the old ones, has to be done file by file since if some component from the generic set is more recent than the specifc ones, it wouldn't be overwritten

install(TARGETS plasmacomponentsplugin DESTINATION ${QML_INSTALL_DIR}/../platformqml/touch/org/kde/plasma/components)

install(FILES qml/BusyIndicator.qml DESTINATION ${QML_INSTALL_DIR}/../platformqml/touch/org/kde/plasma/components)
install(FILES qml/ButtonColumn.qml DESTINATION ${QML_INSTALL_DIR}/../platformqml/touch/org/kde/plasma/components)
install(FILES qml/ButtonGroup.js DESTINATION ${QML_INSTALL_DIR}/../platformqml/touch/org/kde/plasma/components)
install(FILES qml/Button.qml DESTINATION ${QML_INSTALL_DIR}/../platformqml/touch/org/kde/plasma/components)
install(FILES qml/ButtonRow.qml DESTINATION ${QML_INSTALL_DIR}/../platformqml/touch/org/kde/plasma/components)
install(FILES qml/CheckBox.qml DESTINATION ${QML_INSTALL_DIR}/../platformqml/touch/org/kde/plasma/components)
install(FILES qml/CommonDialog.qml DESTINATION ${QML_INSTALL_DIR}/../platformqml/touch/org/kde/plasma/components)
install(FILES qml/Dialog.qml DESTINATION ${QML_INSTALL_DIR}/../platformqml/touch/org/kde/plasma/components)
install(FILES qml/Highlight.qml DESTINATION ${QML_INSTALL_DIR}/../platformqml/touch/org/kde/plasma/components)
install(FILES qml/Label.qml DESTINATION ${QML_INSTALL_DIR}/../platformqml/touch/org/kde/plasma/components)
install(FILES qml/ListItem.qml DESTINATION ${QML_INSTALL_DIR}/../platformqml/touch/org/kde/plasma/components)
install(FILES qml/Page.qml DESTINATION ${QML_INSTALL_DIR}/../platformqml/touch/org/kde/plasma/components)
install(FILES qml/PageStack.qml DESTINATION ${QML_INSTALL_DIR}/../platformqml/touch/org/kde/plasma/components)
install(FILES qml/ProgressBar.qml DESTINATION ${QML_INSTALL_DIR}/../platformqml/touch/org/kde/plasma/components)
install(FILES qml/QueryDialog.qml DESTINATION ${QML_INSTALL_DIR}/../platformqml/touch/org/kde/plasma/components)
install(FILES qml/RadioButton.qml DESTINATION ${QML_INSTALL_DIR}/../platformqml/touch/org/kde/plasma/components)
install(FILES qml/SelectionDialog.qml DESTINATION ${QML_INSTALL_DIR}/../platformqml/touch/org/kde/plasma/components)
install(FILES qml/Slider.qml DESTINATION ${QML_INSTALL_DIR}/../platformqml/touch/org/kde/plasma/components)
install(FILES qml/TabBar.qml DESTINATION ${QML_INSTALL_DIR}/../platformqml/touch/org/kde/plasma/components)
install(FILES qml/TabButton.qml DESTINATION ${QML_INSTALL_DIR}/../platformqml/touch/org/kde/plasma/components)
install(FILES qml/TabGroup.qml DESTINATION ${QML_INSTALL_DIR}/../platformqml/touch/org/kde/plasma/components)
#install(FILES qml/TextArea.qml DESTINATION ${QML_INSTALL_DIR}/../platformqml/touch/org/kde/plasma/components)
#install(FILES qml/TextField.qml DESTINATION ${QML_INSTALL_DIR}/../platformqml/touch/org/kde/plasma/components)
install(FILES qml/ToolBarLayout.qml DESTINATION ${QML_INSTALL_DIR}/../platformqml/touch/org/kde/plasma/components)
install(FILES qml/ToolBar.qml DESTINATION ${QML_INSTALL_DIR}/../platformqml/touch/org/kde/plasma/components)
install(FILES qml/ToolButton.qml DESTINATION ${QML_INSTALL_DIR}/../platformqml/touch/org/kde/plasma/components)


#Now install the private stuff!
install(FILES qml/private/DualStateButton.qml DESTINATION ${QML_INSTALL_DIR}/../platformqml/touch/org/kde/plasma/components/private)
install(FILES qml/private/InlineDialog.qml DESTINATION ${QML_INSTALL_DIR}/../platformqml/touch/org/kde/plasma/components/private)
install(FILES qml/private/PageStack.js DESTINATION ${QML_INSTALL_DIR}/../platformqml/touch/org/kde/plasma/components/private)
install(FILES qml/private/TabGroup.js DESTINATION ${QML_INSTALL_DIR}/../platformqml/touch/org/kde/plasma/components/private)
install(FILES qml/private/ScrollBarDelegate.qml DESTINATION ${QML_INSTALL_DIR}/../platformqml/touch/org/kde/plasma/components/private)
install(FILES qml/private/ScrollDecoratorDelegate.qml DESTINATION ${QML_INSTALL_DIR}/../platformqml/touch/org/kde/plasma/components/private)
install(FILES qml/private/SectionScroller.js DESTINATION ${QML_INSTALL_DIR}/../platformqml/touch/org/kde/plasma/components/private)
install(FILES qml/private/AppManager.js DESTINATION ${QML_INSTALL_DIR}/../platformqml/touch/org/kde/plasma/components/private)
install(FILES qml/private/TabBarLayout.qml DESTINATION ${QML_INSTALL_DIR}/../platformqml/touch/org/kde/plasma/components/private)
install(FILES qml/private/TextFieldFocus.qml DESTINATION ${QML_INSTALL_DIR}/../platformqml/touch/org/kde/plasma/components/private)

#install platform overrides
install(DIRECTORY platformcomponents/touch/ DESTINATION ${QML_INSTALL_DIR}/../platformqml/touch/org/kde/plasma/components)
