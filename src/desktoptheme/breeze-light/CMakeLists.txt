configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/metadata.json.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/metadata.json"
    @ONLY
)

install(
    FILES colors "${CMAKE_CURRENT_BINARY_DIR}/metadata.json"
          plasmarc
    DESTINATION ${PLASMA_DATA_INSTALL_DIR}/desktoptheme/breeze-light
)
