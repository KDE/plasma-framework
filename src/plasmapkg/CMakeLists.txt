project("plasmapkg")

add_executable(plasmapkg
    main.cpp
    plasmapkg.cpp
)

target_link_libraries(plasmapkg kdeqt5staging plasma ${KI18n_LIBRARIES} ${KService_LIBRARIES} )

message("INSTALL_TARGETS_DEFAULT_ARGS ${INSTALL_TARGETS_DEFAULT_ARGS}")

install(TARGETS plasmapkg ${INSTALL_TARGETS_DEFAULT_ARGS})

