
install(FILES colors metadata.desktop DESTINATION ${PLASMA_DATA_INSTALL_DIR}/desktoptheme/breeze-dark/)

FILE(GLOB translucent_widgets translucent/widgets/*.svgz)
install( FILES ${translucent_widgets} DESTINATION ${PLASMA_DATA_INSTALL_DIR}/desktoptheme/breeze-dark/translucent/widgets/ )
