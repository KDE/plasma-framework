set(plasmapkg_SRCS
    main.cpp
)

include_directories(${KDE4_INCLUDE_DIR}/KDE)
kde4_add_executable(plasmapkg ${plasmapkg_SRCS})

target_link_libraries(plasmapkg ${KDE4_KDEUI_LIBS} ${KDE4_PLASMA_LIBS})

install(TARGETS plasmapkg ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES data/knewstuff/plasmoids.knsrc DESTINATION  ${CONFIG_INSTALL_DIR})


