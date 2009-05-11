set(plasmapkg_SRCS
    main.cpp
)

kde4_add_executable(plasmapkg ${plasmapkg_SRCS})

target_link_libraries(plasmapkg ${KDE4_KDEUI_LIBS} ${KDE4_PLASMA_LIBS})

install(TARGETS plasmapkg ${INSTALL_TARGETS_DEFAULT_ARGS})

