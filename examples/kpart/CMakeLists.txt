set(plasma_example_kpart_shell_SRCS
   main.cpp
   containmentshell.cpp
   testshellpluginloader.cpp
   appletselector.cpp
)

kde4_add_ui_files(plasma_example_kpart_shell_SRCS appletselector.ui)
kde4_add_executable(plasma-example-kpart-shell ${plasma_example_kpart_shell_SRCS})

target_link_libraries(plasma-example-kpart-shell ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_PLASMA_LIBS} )

########### install files ###############
install(TARGETS plasma-example-kpart-shell ${INSTALL_TARGETS_DEFAULT_ARGS} )
install( PROGRAMS plasma-example-kpart-shell.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install(FILES plasma-default-layoutrc DESTINATION ${DATA_INSTALL_DIR}/plasma-example-kpart-shell/)
install( FILES plasma-kpart-shellui.rc  DESTINATION ${DATA_INSTALL_DIR}/plasma-example-kpart-shell/ )
