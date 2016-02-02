
install(FILES colors metadata.desktop DESTINATION ${PLASMA_DATA_INSTALL_DIR}/desktoptheme/air/)

FILE(GLOB widgets widgets/*.svgz)
install( FILES ${widgets} DESTINATION ${PLASMA_DATA_INSTALL_DIR}/desktoptheme/air/widgets/ )

FILE(GLOB dialogs dialogs/*.svgz)
install( FILES ${dialogs} DESTINATION ${PLASMA_DATA_INSTALL_DIR}/desktoptheme/air/dialogs/ )

FILE(GLOB opaque_dialogs opaque/dialogs/*.svgz)
install( FILES ${opaque_dialogs} DESTINATION ${PLASMA_DATA_INSTALL_DIR}/desktoptheme/air/opaque/dialogs/ )

FILE(GLOB opaque_widgets opaque/widgets/*.svgz)
install( FILES ${opaque_widgets} DESTINATION ${PLASMA_DATA_INSTALL_DIR}/desktoptheme/air/opaque/widgets/ )

FILE(GLOB translucent_widgets translucent/widgets/*.svgz)
install( FILES ${translucent_widgets} DESTINATION ${PLASMA_DATA_INSTALL_DIR}/desktoptheme/air/translucent/widgets/ )

FILE(GLOB translucent_dialogs translucent/dialogs/*.svgz)
install( FILES ${translucent_dialogs} DESTINATION ${PLASMA_DATA_INSTALL_DIR}/desktoptheme/air/translucent/dialogs/ )


