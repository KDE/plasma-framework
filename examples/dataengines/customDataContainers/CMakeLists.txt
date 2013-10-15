set(customDataContainers_SRCS
    customDataContainersEngine.cpp
    httpContainer.cpp
)

kservice_desktop_to_json(plasma-dataengine-example-customDataContainers.desktop)

plasma_add_plugin(plasma_dataengine_example_customDataContainers ${customDataContainers_SRCS})
target_link_libraries(plasma_dataengine_example_customDataContainers
  Plasma
  KF5::KIOCore
  KF5::KService
)

install(TARGETS plasma_dataengine_example_customDataContainers DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-example-customDataContainers.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
