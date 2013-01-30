set(plasma_remote_helper_SRCS
    main.cpp
)

kde4_add_executable(plasma-remote-helper ${plasma_remote_helper_SRCS})

target_link_libraries(plasma-remote-helper ${KDE4_KDEUI_LIBS})

install(TARGETS plasma-remote-helper ${INSTALL_TARGETS_DEFAULT_ARGS})

