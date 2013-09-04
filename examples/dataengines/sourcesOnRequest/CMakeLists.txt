set(sourcesOnRequest_SRCS
    sourcesOnRequest.cpp
)

kservice_desktop_to_json(plasma-dataengine-example-sourcesOnRequest.desktop)

plasma_add_plugin(plasma_dataengine_example_sourcesOnRequest ${sourcesOnRequest_SRCS})

target_link_libraries(plasma_dataengine_example_sourcesOnRequest
  plasma
  ${KService_LIBRARIES}
)

install(TARGETS plasma_dataengine_example_sourcesOnRequest DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-example-sourcesOnRequest.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
