
install(FILES colors metadata.desktop DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/air/)

FILE(GLOB widgets widgets/*.svgz)
install( FILES ${widgets} DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/air/widgets/ )

FILE(GLOB dialogs dialogs/*.svgz)
install( FILES ${dialogs} DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/air/dialogs/ )

FILE(GLOB opaque_dialogs opaque/dialogs/*.svgz)
install( FILES ${opaque_dialogs} DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/air/opaque/dialogs/ )

FILE(GLOB opaque_widgets opaque/widgets/*.svgz)
install( FILES ${opaque_widgets} DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/air/opaque/widgets/ )
