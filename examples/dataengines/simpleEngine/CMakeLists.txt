set(simpleEngine_SRCS
    simpleEngine.cpp
)

kde4_add_plugin(plasma_dataengine_example_simpleEngine ${simpleEngine_SRCS})
target_link_libraries(plasma_dataengine_example_simpleEngine ${KDE4_PLASMA_LIBS})

install(TARGETS plasma_dataengine_example_simpleEngine DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-example-simpleEngine.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
