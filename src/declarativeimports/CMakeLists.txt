add_subdirectory(accessdenied)
add_subdirectory(core)
add_subdirectory(plasmacomponents)
add_subdirectory(plasmaextracomponents)
add_subdirectory(platformcomponents)
add_subdirectory(calendar)

install(DIRECTORY plasmastyle/ DESTINATION ${KDE_INSTALL_QMLDIR}/QtQuick/Controls/Styles/Plasma)
install(DIRECTORY qqc2style/ DESTINATION ${KDE_INSTALL_QMLDIR}/QtQuick/Controls.2/Plasma)
