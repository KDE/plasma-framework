set(simpleEngine_SRCS
    simpleEngine.cpp
)

add_library(plasma_dataengine_example_simpleEngine MODULE ${simpleEngine_SRCS})

kservice_desktop_to_json(plasma_dataengine_example_simpleEngine plasma-dataengine-example-simpleEngine.desktop)

target_link_libraries(plasma_dataengine_example_simpleEngine
  KF5::Plasma
  KF5::Service
  KF5::I18n
)

install(TARGETS plasma_dataengine_example_simpleEngine DESTINATION ${PLUGIN_INSTALL_DIR}/plasma/dataengine)
install(FILES plasma-dataengine-example-simpleEngine.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
