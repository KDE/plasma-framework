set(sourcesOnRequest_SRCS
    sourcesOnRequest.cpp
)

kservice_desktop_to_json(plasma-dataengine-example-sourcesOnRequest.desktop)

add_library(plasma_dataengine_example_sourcesOnRequest MODULE ${sourcesOnRequest_SRCS})

target_link_libraries(plasma_dataengine_example_sourcesOnRequest
  KF5::Plasma
  KF5::Service
)

install(TARGETS plasma_dataengine_example_sourcesOnRequest DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-example-sourcesOnRequest.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
