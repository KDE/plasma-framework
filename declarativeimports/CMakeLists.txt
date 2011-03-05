add_subdirectory(core)
add_subdirectory(graphicslayouts)
add_subdirectory(graphicswidgets)
add_subdirectory(qtextracomponents)

install(DIRECTORY plasmacomponents/ DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/plasma/components)