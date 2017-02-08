add_executable(plasmapkg2 main.cpp)

target_link_libraries(plasmapkg2 Qt5::Core)

install(TARGETS plasmapkg2 ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

