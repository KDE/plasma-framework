configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/metadata.desktop.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/metadata.desktop"
    @ONLY
)

install(
    FILES colors "${CMAKE_CURRENT_BINARY_DIR}/metadata.desktop"
    DESTINATION ${PLASMA_DATA_INSTALL_DIR}/desktoptheme/air
)

FILE(GLOB widgets widgets/*.svg)
plasma_install_desktoptheme_svgs(air SUBPATH widgets FILES ${widgets})

FILE(GLOB dialogs dialogs/*.svg)
plasma_install_desktoptheme_svgs(air SUBPATH dialogs FILES ${dialogs})

FILE(GLOB opaque_dialogs opaque/dialogs/*.svg)
plasma_install_desktoptheme_svgs(air SUBPATH opaque/dialogs FILES ${opaque_dialogs})

FILE(GLOB opaque_widgets opaque/widgets/*.svg)
plasma_install_desktoptheme_svgs(air SUBPATH opaque/widgets FILES ${opaque_widgets})

FILE(GLOB translucent_widgets translucent/widgets/*.svg)
plasma_install_desktoptheme_svgs(air SUBPATH translucent/widgets FILES ${translucent_widgets})

FILE(GLOB translucent_dialogs translucent/dialogs/*.svg)
plasma_install_desktoptheme_svgs(air SUBPATH translucent/dialogs FILES ${translucent_dialogs})

FILE(GLOB icons icons/*.svg)
plasma_install_desktoptheme_svgs(air SUBPATH icons FILES ${icons})

