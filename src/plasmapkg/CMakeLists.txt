add_executable(plasmapkg2 main.cpp)

target_link_libraries(plasmapkg2 Qt${QT_MAJOR_VERSION}::Core)

ecm_mark_nongui_executable(plasmapkg2)

install(TARGETS plasmapkg2 ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

