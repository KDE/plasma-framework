project(plasmaextracomponents)

find_package(KActivities QUIET CONFIG)
set_package_properties(KActivities PROPERTIES DESCRIPTION "Library and infrastructure for using Activities from applications"
                       URL "https://projects.kde.org/kactivities"
                       TYPE REQUIRED
                       PURPOSE "Provides the ability to access activities from QML."
                      )

set(plasmaextracomponents_SRCS
    appbackgroundprovider.cpp
    resourceinstance.cpp
    plasmaextracomponentsplugin.cpp
    fallbackcomponent.cpp
    )

include_directories(
        ${KACTIVITIES_INCLUDE_DIRS}
)

qt4_automoc(${plasmaextracomponents_SRCS})


add_library(plasmaextracomponentsplugin SHARED ${plasmaextracomponents_SRCS})
target_link_libraries(plasmaextracomponentsplugin ${QT_QTCORE_LIBRARY} ${QT_QTDECLARATIVE_LIBRARY}
                                                    ${QT_QTGUI_LIBRARY} ${KDE4_PLASMA_LIBS} ${KACTIVITIES_LIBRARY} )

install(TARGETS plasmaextracomponentsplugin DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/plasma/extras)

install(DIRECTORY qml/ DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/plasma/extras)


# The platform specific stuff, overwrites a copy of the desktop one
# it does install some files on top of the old ones, has to be done
# file by file since if some component from the generic set is more
# recent than the specifc ones, it wouldn't be overwritten

#install(TARGETS plasmaextracomponentsplugin DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/extras)
#install(FILES qml/qmldir DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/extras)

# install(FILES qml/App.qml DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/extras)
# install(FILES qml/Heading.qml DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/extras)
# install(FILES qml/Paragraph.qml DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/extras)
# install(FILES qml/Title.qml DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/extras)
# install(DIRECTORY qml/animations/ DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/extras)

#install(DIRECTORY qml/animations/ DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/plasma/extras)

#install platform overrides -- enable as we use this feature for plasmaextras
#install(DIRECTORY platformcomponents/touch/ DESTINATION ${PLUGIN_INSTALL_DIR}/platformimports/touch/org/kde/plasma/extras)
