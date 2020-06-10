configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/metadata.desktop.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/metadata.desktop"
    @ONLY
)

install(
    FILES colors "${CMAKE_CURRENT_BINARY_DIR}/metadata.desktop"
    DESTINATION ${PLASMA_DATA_INSTALL_DIR}/desktoptheme/breeze-dark
)
install(
    DIRECTORY widgets
    DESTINATION ${PLASMA_DATA_INSTALL_DIR}/desktoptheme/breeze-dark
)
