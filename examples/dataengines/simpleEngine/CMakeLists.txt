set(simpleEngine_SRCS
    simpleEngine.cpp
)

kservice_desktop_to_json(plasma-dataengine-example-simpleEngine.desktop)

add_library(plasma_dataengine_example_simpleEngine MODULE ${simpleEngine_SRCS})

target_link_libraries(plasma_dataengine_example_simpleEngine
  KF5::Plasma
  KF5::Service
  KF5::I18n
)

install(TARGETS plasma_dataengine_example_simpleEngine DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-example-simpleEngine.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
