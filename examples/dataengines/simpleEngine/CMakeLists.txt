set(simpleEngine_SRCS
    simpleEngine.cpp
)

kservice_desktop_to_json(plasma-dataengine-example-simpleEngine.desktop)

plasma_add_plugin(plasma_dataengine_example_simpleEngine ${simpleEngine_SRCS})

target_link_libraries(plasma_dataengine_example_simpleEngine
  plasma
  ${KService_LIBRARIES}
  ${KI18n_LIBRARIES}
)

install(TARGETS plasma_dataengine_example_simpleEngine DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-example-simpleEngine.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
