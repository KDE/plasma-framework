configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/metadata.desktop.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/metadata.desktop"
    @ONLY
)

install(
    FILES colors "${CMAKE_CURRENT_BINARY_DIR}/metadata.desktop"
    DESTINATION ${PLASMA_DATA_INSTALL_DIR}/desktoptheme/breeze-light
)
install(
    FILES ${PROJECT_BINARY_DIR}/src/desktoptheme/breeze/default.gzipped/widgets/plasmoidheading.svgz
    DESTINATION ${PLASMA_DATA_INSTALL_DIR}/desktoptheme/breeze-light/widgets
)
