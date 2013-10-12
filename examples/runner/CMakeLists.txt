# Project Needs a name, of course
project(RunnerExample)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(${KDE4_INCLUDES})

# We add our source code here
set(example_SRCS homefilesrunner.cpp)

# Now make sure all files get to the right place
kde4_add_plugin(plasma_runner_example_homefiles ${example_SRCS})
target_link_libraries(plasma_runner_example_homefiles ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS})

# Install the library and .desktop file
install(TARGETS plasma_runner_example_homefiles DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-runner-example-homefiles.desktop DESTINATION ${SERVICES_INSTALL_DIR})
