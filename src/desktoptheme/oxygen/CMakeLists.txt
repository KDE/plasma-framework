
install(FILES colors metadata.desktop DESTINATION ${PLASMA_DATA_INSTALL_DIR}/desktoptheme/oxygen/)

FILE(GLOB widgets widgets/*.svg)
plasma_install_desktoptheme_svgs(oxygen SUBPATH widgets FILES ${widgets})

FILE(GLOB dialogs dialogs/*.svg)
plasma_install_desktoptheme_svgs(oxygen SUBPATH dialogs FILES ${dialogs})

FILE(GLOB opaque opaque/widgets/*.svg)
plasma_install_desktoptheme_svgs(oxygen SUBPATH opaque/widgets FILES ${opaque})

FILE(GLOB opaque opaque/dialogs/*.svg)
plasma_install_desktoptheme_svgs(oxygen SUBPATH opaque/dialogs FILES ${opaque})

FILE(GLOB icons ../air/icons/*.svg)
plasma_install_desktoptheme_svgs(oxygen SUBPATH icons FILES ${icons})

