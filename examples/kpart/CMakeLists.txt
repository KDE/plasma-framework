set(plasma_example_kpart_shell_SRCS
   main.cpp
   containmentshell.cpp
   testshellpluginloader.cpp
   appletselector.cpp
)

ki18n_wrap_ui(plasma_example_kpart_shell_SRCS appletselector.ui)
add_executable(plasma-example-kpart-shell ${plasma_example_kpart_shell_SRCS})

target_link_libraries(plasma-example-kpart-shell KF5::Plasma KF5::Parts)

########### install files ###############
install(TARGETS plasma-example-kpart-shell ${KF5_INSTALL_TARGETS_DEFAULT_ARGS} )
install( PROGRAMS plasma-example-kpart-shell.desktop DESTINATION ${KDE_INSTALL_APPDIR} )
install(FILES plasma-default-layoutrc DESTINATION ${KDE_INSTALL_DATADIR}/plasma-example-kpart-shell/)
install( FILES plasma-kpart-shellui.rc  DESTINATION ${KDE_INSTALL_DATADIR}/plasma-example-kpart-shell/ )
