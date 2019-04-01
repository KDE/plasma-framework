
install(FILES metadata.desktop DESTINATION ${PLASMA_DATA_INSTALL_DIR}/desktoptheme/default/)

FILE(GLOB widgets widgets/*.svg)
plasma_install_desktoptheme_svgs(default SUBPATH widgets FILES ${widgets})

FILE(GLOB dialogs dialogs/*.svg)
plasma_install_desktoptheme_svgs(default SUBPATH dialogs FILES ${dialogs})

FILE(GLOB opaque_dialogs opaque/dialogs/*.svg)
plasma_install_desktoptheme_svgs(default SUBPATH opaque/dialogs FILES ${opaque_dialogs})

FILE(GLOB opaque_widgets opaque/widgets/*.svg)
plasma_install_desktoptheme_svgs(default SUBPATH opaque/widgets FILES ${opaque_widgets})

FILE(GLOB translucent_widgets translucent/widgets/*.svg)
plasma_install_desktoptheme_svgs(default SUBPATH translucent/widgets FILES ${translucent_widgets})

FILE(GLOB translucent_dialogs translucent/dialogs/*.svg)
plasma_install_desktoptheme_svgs(default SUBPATH translucent/dialogs FILES ${translucent_dialogs})

FILE(GLOB icons icons/*.svg)
plasma_install_desktoptheme_svgs(default SUBPATH icons FILES ${icons})

