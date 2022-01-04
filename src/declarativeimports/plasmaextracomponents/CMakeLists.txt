# find_package(KActivities QUIET CONFIG)
# set_package_properties(KActivities PROPERTIES DESCRIPTION "Library and infrastructure for using Activities from applications"
#                        URL "https://commits.kde.org/kactivities"
#                        TYPE REQUIRED
#                        PURPOSE "Provides the ability to access activities from QML."
#                       )
add_library(plasmaextracomponentsplugin SHARED)

target_sources(plasmaextracomponentsplugin PRIVATE
    appbackgroundprovider.cpp
    plasmaextracomponentsplugin.cpp
    fallbackcomponent.cpp
)

target_link_libraries(plasmaextracomponentsplugin
        Qt${QT_MAJOR_VERSION}::Quick
        Qt${QT_MAJOR_VERSION}::Qml
        ${KACTIVITIES_LIBRARY}
        KF5::Service
        KF5::Plasma)


install(TARGETS plasmaextracomponentsplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/extras)

install(DIRECTORY qml/ DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/extras)

ecm_generate_qmltypes(org.kde.plasma.extras 2.0 DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/extras)


# The platform specific stuff, overwrites a copy of the desktop one
# it does install some files on top of the old ones, has to be done
# file by file since if some component from the generic set is more
# recent than the specific ones, it wouldn't be overwritten

#install(TARGETS plasmaextracomponentsplugin DESTINATION ${KDE_INSTALL_PLUGINDIR}/platformimports/touch/org/kde/plasma/extras)
#install(FILES qml/qmldir DESTINATION ${KDE_INSTALL_PLUGINDIR}/platformimports/touch/org/kde/plasma/extras)

# install(FILES qml/App.qml DESTINATION ${KDE_INSTALL_PLUGINDIR}/platformimports/touch/org/kde/plasma/extras)
# install(FILES qml/Heading.qml DESTINATION ${KDE_INSTALL_PLUGINDIR}/platformimports/touch/org/kde/plasma/extras)
# install(FILES qml/Paragraph.qml DESTINATION ${KDE_INSTALL_PLUGINDIR}/platformimports/touch/org/kde/plasma/extras)
# install(FILES qml/Title.qml DESTINATION ${KDE_INSTALL_PLUGINDIR}/platformimports/touch/org/kde/plasma/extras)
# install(DIRECTORY qml/animations/ DESTINATION ${KDE_INSTALL_PLUGINDIR}/platformimports/touch/org/kde/plasma/extras)

#install(DIRECTORY qml/animations/ DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/extras)

#install platform overrides -- enable as we use this feature for plasmaextras
#install(DIRECTORY platformcomponents/touch/ DESTINATION ${KDE_INSTALL_PLUGINDIR}/platformimports/touch/org/kde/plasma/extras)
