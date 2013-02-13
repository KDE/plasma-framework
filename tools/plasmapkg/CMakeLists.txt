project("plasmapkg")

add_executable(plasmapkg
    main.cpp
    plasmapkg.cpp
)

target_link_libraries(plasmapkg ${KDE4_KDECORE_LIBS} plasma)

message("INSTALL_TARGETS_DEFAULT_ARGS ${INSTALL_TARGETS_DEFAULT_ARGS}")

install(TARGETS plasmapkg ${INSTALL_TARGETS_DEFAULT_ARGS})

